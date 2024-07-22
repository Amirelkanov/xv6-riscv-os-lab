#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "check_helpers.c"
#include "lab2_checkers.c"
#include "lab7_checkers.c"

const char *first_file_abs_path = "/d1/d2/lns/1",
           *second_file_abs_path = "/d1/2",
           *third_file_abs_path = "/d1/d2/lns/ln1/ln2/3";

void preprocess_dirs_n_files()
{
    safe_mkdir("d1");
    safe_mkdir("d1/d2");
    safe_mkdir("d1/d2/lns");
    safe_mkdir("d1/d2/lns/ln1");
    safe_mkdir("d1/d2/lns/ln1/ln2");

    safe_create_file(first_file_abs_path, '1');
    safe_create_file(second_file_abs_path, '2');
    safe_create_file(third_file_abs_path, '3');
}

void preprocess_symlinks()
{
    // a - абсолютные сим. ссылки, o - относительные сим. ссылки

    // Корректная абсолютная ссылка на файл
    safe_symlink(first_file_abs_path, "/d1/d2/lns/a1");

    // Корректная абсолютная ссылка на абсолютную символическую ссылку (глубина рекурсии - 2)
    safe_symlink("/d1/d2/lns/a1", "/d1/d2/lns/aa1");
    safe_symlink("/d1/d2/lns/aa1", "/d1/d2/lns/aa2");

    // Корректная абсолютная ссылка на относительную символическую ссылку (глубина рекурсии - 2)
    safe_symlink("/d1/d2/lns/o1", "/d1/d2/lns/oa1"); // o1 - будущая относительная символическая ссылка
    safe_symlink("/d1/d2/lns/oa1", "/d1/d2/lns/oa2");

    safe_chdir("d1/d2/lns");

    // Корректная относительная ссылка на файл того же каталога
    safe_symlink("./1", "o1");
    // Корректная относительная ссылка на файл каталога на 2 уровня выше
    safe_symlink("../../2", "o2");
    // Корректная относительная ссылка на файл каталога на 2 уровня ниже
    safe_symlink("ln1/ln2/3", "o3");

    // Корректная относительная ссылка на относительную символическую ссылку (глубина рекурсии - 2)
    safe_symlink("o1", "oo1");
    safe_symlink("oo1", "oo2");

    // Ссылка на себя (бесконечная рекурсия)
    safe_symlink("self", "self");

    // Косвенная ссылка на себя (бесконечная рекурсия через 2-3 перехода)
    safe_symlink("indir", "indir1");
    safe_symlink("indir1", "indir2");
    safe_symlink("indir2", "indir");

    // Абсолютная ссылка на несуществующий файл
    safe_symlink("/d1/d2/lns/4", "/d1/d2/lns/err");

    // Относительная ссылка на несуществующий файл того же каталога
    // (при этом файл существует в каталоге на 2 уровня выше)
    safe_symlink("2", "errcur");

    // Относительная ссылка на несуществующий файл каталога на 2 уровня выше/ниже (при этом файл в том же каталоге существует)
    safe_symlink("../../1", "errup");
    safe_symlink("ln1/ln2/1", "errdwn");
}

void preprocess()
{
    preprocess_dirs_n_files();
    preprocess_symlinks();
}

void postprocess()
{
    // Удаляем все символические ссылки
    safe_unlink("a1");
    safe_unlink("aa1");
    safe_unlink("aa2");
    safe_unlink("oa1");
    safe_unlink("oa2");

    safe_unlink("o1");
    safe_unlink("o2");
    safe_unlink("o3");
    safe_unlink("oo1");
    safe_unlink("oo2");
    safe_unlink("self");
    safe_unlink("indir1");
    safe_unlink("indir2");
    safe_unlink("indir");
    safe_unlink("err");
    safe_unlink("errcur");
    safe_unlink("errup");
    safe_unlink("errdwn");

    // Удаляем файлы
    safe_unlink(first_file_abs_path);
    safe_unlink(second_file_abs_path);
    safe_unlink(third_file_abs_path);

    safe_chdir("../../../../");

    // Удаляем директории
    safe_unlink("d1/d2/lns/ln1/ln2");
    safe_unlink("d1/d2/lns/ln1");
    safe_unlink("d1/d2/lns");
    safe_unlink("d1/d2");
    safe_unlink("d1");
}

void test_correct_abs_symlinks()
{
    printf("Correct absolute symlinks: ");
    check(first_file_abs_path, "a1", 0);
    check(first_file_abs_path, "aa2", 0);
    check(first_file_abs_path, "oa2", 0);
    printf("OK\n");
}

void test_correct_rel_symlinks()
{
    printf("Correct relative symlinks: ");
    check(first_file_abs_path, "o1", 0);
    check(second_file_abs_path, "o2", 0);
    check(third_file_abs_path, "o3", 0);
    check(first_file_abs_path, "oo1", 0);
    printf("OK\n");
}

void test_incorrect_symlinks()
{
    printf("Incorrect symlinks: ");
    check("self", "self", 1);
    check("indir", "indir2", 1);
    check("/d1/d2/lns/4", "err", 1);
    check("2", "errcur", 1);
    check("../../1", "errup", 1);
    check("ln1/ln2/1", "errdwn", 1);
    printf("OK\n");
}

void run_tests()
{
    printf("===== Symlink tests started =====\n");
    test_correct_abs_symlinks();
    test_correct_rel_symlinks();
    test_incorrect_symlinks();
    printf("===== Symlink tests finished =====\n");
}

void print_dir_structure(char *dir, char *ls_path)
{
    int pid = fork();
    fork_check(pid);

    if (pid == 0)
    {
        printf("===================\n%s\n===================\n", dir);
        safe_chdir(dir);

        char *argv[] = {0};
        exec(ls_path, argv);

        raise_err("exec ls failed\n");
    }
    else wait(0);
}

void print_test_dirs()
{
    printf("\n===== Test directories structure =====\n\n");
    print_dir_structure("/", "ls");
    print_dir_structure("/d1", "../ls");
    print_dir_structure("/d1/d2", "../../ls");
    print_dir_structure("/d1/d2/lns", "../../../ls");
    print_dir_structure("/d1/d2/lns/ln1", "../../../../ls");
    print_dir_structure("/d1/d2/lns/ln1/ln2", "../../../../../ls");
    printf("\n===== Test directories structure finished =====\n");
}

int main()
{
    preprocess();
    run_tests();
    print_test_dirs();
    postprocess();

    exit(0);
}