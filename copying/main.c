/* splice / copy_file_range */
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <inttypes.h>

/* sendfile */
#include <sys/sendfile.h>

/* copy_file_range */
#include <sys/syscall.h>
#include <unistd.h>

static ssize_t
copy_file_range(int fd_in, loff_t *off_in,
                int fd_out, loff_t *off_out,
                size_t len, unsigned int flags)
{
    return syscall(__NR_copy_file_range,
                   fd_in, off_in, fd_out, off_out, len, flags);
}

struct in_pipe_data
{
    FILE *process;
};

struct out_pipe_data
{
    pid_t pid;
};

struct test_case
{
    const char *name;

    const char *src_file;
    int src_fd;

    const char *dst_file;
    int dst_fd;

    struct in_pipe_data in_pipe;
    struct out_pipe_data out_pipe;

    void (*open_src)(struct test_case *test);
    void (*open_dst)(struct test_case *test);

    void (*handle_result)(struct test_case *test, const char *fn, ssize_t res, int eno);

    void (*close_src)(struct test_case *test);
    void (*close_dst)(struct test_case *test);
};

static void
tc_start(struct test_case *tc)
{
    printf("Starting test: %s\n", tc->name);
}

static void
tc_finish(struct test_case *tc)
{
    printf("Finished test: %s\n", tc->name);
}

static void
tc_print_result(struct test_case *tc, const char *function, ssize_t res, int ferrno)
{
    const char *status = "[FAILED]";
    if (res == 0) {
        struct stat st;
        if (stat(tc->dst_file, &st) < 0) {
            status = "[BADFILE]";
        }
        else {
            status = (st.st_size != 0) ? "[PASSED]" : "[BADCOPY]";
        }
    }

    printf("%20s %-9s %3d %s\n", function, status, ferrno, strerror(ferrno));
}

static void
tc_dst_pipe_handle_result(struct test_case *tc, const char *function, ssize_t res, int ferrno)
{
    close(tc->dst_fd);

    int status = 0;
    if (waitpid(tc->out_pipe.pid, &status, 0) < 0) {
        warn("Waiting for child %d failed", tc->out_pipe.pid);
    }

    if (WIFSIGNALED(status)) {
        warn("Child killed with %d", WTERMSIG(status));
    }

    if (WIFEXITED(status) && WEXITSTATUS(status)) {
        warn("Child exited with %d", WEXITSTATUS(status));
    }

    if (!WIFEXITED(status)) {
        warn("Child not exited");
    }

    tc_print_result(tc, function, res, ferrno);
}

static void
tc_open_src_file(struct test_case *tc)
{
    tc->src_fd = open(tc->src_file, O_RDONLY);
    if (tc->src_fd < 0) {
        err(EXIT_FAILURE, "Failed to open source file '%s'", tc->src_file);
    }
}

static void
tc_close_src_file(struct test_case *tc)
{
    close(tc->src_fd);
}

static void
tc_open_src_pipe(struct test_case *tc)
{
    static const char *const fmt = "/usr/bin/cat %s";
    const size_t len = strlen(fmt) + strlen(tc->src_file) + 1;

    char *cmd = (char *)malloc(len);
    if (cmd == NULL) {
        err(EXIT_FAILURE, "malloc");
    }

    const int wrote = snprintf(cmd, len, "/usr/bin/cat %s", tc->src_file);
    if (wrote < 0) {
        err(EXIT_FAILURE, "snprintf('/usr/bin/cat %%s', %s)", tc->src_file);
    }
    if ((size_t)wrote >= len) {
        errx(EXIT_FAILURE, "Too narrow buffer for command '/usr/bin/cat %s'", tc->src_file);
    }

    FILE *process = popen(cmd, "r");
    if (process == NULL) {
        err(EXIT_FAILURE, "popen(%s, 'r')", cmd);
    }

    tc->in_pipe.process = process;
    tc->src_fd = fileno(process);
}

static void
tc_close_src_pipe(struct test_case *tc)
{
    pclose(tc->in_pipe.process);
}

static void
tc_open_dst_file(struct test_case *tc)
{
    unlink(tc->dst_file);
    tc->dst_fd = open(tc->dst_file, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (tc->dst_fd < 0) {
        err(EXIT_FAILURE, "Failed to open destination file '%s'", tc->dst_file);
    }
}

static void
tc_close_dst_file(struct test_case *tc)
{
    close(tc->dst_fd);
    unlink(tc->dst_file);
}

static void
tc_open_dst_pipe(struct test_case *tc)
{
    tc_open_dst_file(tc);

    int pfd[2];
    if (pipe(pfd) < 0) {
        err(EXIT_FAILURE, "pipe");
    }

    pid_t pid = fork();
    if (pid < 0) {
        err(EXIT_FAILURE, "fork");
    }

    if (pid == 0) {
        close(STDIN_FILENO);
        if (dup2(pfd[STDIN_FILENO], STDIN_FILENO) < 0) {
            err(EXIT_FAILURE, "Failed to forward pipe to stdin");
        }

        close(pfd[STDOUT_FILENO]);
        close(STDOUT_FILENO);
        if (dup2(tc->dst_fd, STDOUT_FILENO) < 0) {
            err(EXIT_FAILURE, "Failed to forward stdout to destination file");
        }

        execl("/usr/bin/cat", "cat", NULL);
        err(EXIT_FAILURE, "execl('/usr/bin/cat')");
    }

    close(pfd[STDIN_FILENO]);

    tc->out_pipe.pid = pid;
    tc->dst_fd = pfd[STDOUT_FILENO];
}

#define COPY_BLOCK_SIZE UINT32_MAX

static inline ssize_t
tc_copy_file_range(int src_fd, int dst_fd)
{
    return copy_file_range(src_fd, NULL, dst_fd, NULL, COPY_BLOCK_SIZE, 0);
}

static inline ssize_t
tc_splice(int src_fd, int dst_fd)
{
    return splice(src_fd, NULL, dst_fd, NULL, COPY_BLOCK_SIZE, 0);
}

static inline ssize_t
tc_sendfile(int src_fd, int dst_fd)
{
    return sendfile(dst_fd, src_fd, NULL, COPY_BLOCK_SIZE);
}

static inline ssize_t
tc_read_write(int src_fd, int dst_fd)
{
    char buffer[4096];
    const int r = read(src_fd, buffer, sizeof(buffer));
    if (r > 0)
        return write(dst_fd, buffer, sizeof(buffer));

    return r;
}

#define CONDUCT_TEST(function) \
do { \
    tc->open_src(tc); \
    tc->open_dst(tc); \
    ssize_t res; \
    do { \
        errno = 0; \
        res = tc_##function(tc->src_fd, tc->dst_fd); \
    } while (res > 0); \
    tc->handle_result(tc, #function, res, errno); \
    tc->close_dst(tc); \
    tc->close_src(tc); \
} while (0)

static void
tc_run(struct test_case *tc)
{
    tc_start(tc);

    CONDUCT_TEST(splice);
    CONDUCT_TEST(sendfile);
    CONDUCT_TEST(copy_file_range);
    CONDUCT_TEST(read_write);

    tc_finish(tc);
}

static void
test_regular_files(const char *src_file, const char *dst_file)
{
    struct test_case tc = {
        .name = "Regular files",
        .src_file = src_file,
        .dst_file = dst_file,
        .open_src = tc_open_src_file,
        .open_dst = tc_open_dst_file,
        .handle_result = tc_print_result,
        .close_src = tc_close_src_file,
        .close_dst = tc_close_dst_file,
    };

    tc_run(&tc);
}

static void
test_in_pipe(const char *src_file, const char *dst_file)
{
    struct test_case tc = {
        .name = "In PIPE",
        .src_file = src_file,
        .dst_file = dst_file,
        .open_src = tc_open_src_pipe,
        .open_dst = tc_open_dst_file,
        .handle_result = tc_print_result,
        .close_src = tc_close_src_pipe,
        .close_dst = tc_close_dst_file,
    };

    tc_run(&tc);
}

static void
test_out_pipe(const char *src_file, const char *dst_file)
{
    struct test_case tc = {
        .name = "Out PIPE",
        .src_file = src_file,
        .dst_file = dst_file,
        .open_src = tc_open_src_file,
        .open_dst = tc_open_dst_pipe,
        .handle_result = tc_dst_pipe_handle_result,
        .close_src = tc_close_src_file,
        .close_dst = tc_close_dst_file,
    };

    tc_run(&tc);
}

static void
test_both_pipe(const char *src_file, const char *dst_file)
{
    struct test_case tc = {
        .name = "Both PIPE",
        .src_file = src_file,
        .dst_file = dst_file,
        .open_src = tc_open_src_pipe,
        .open_dst = tc_open_dst_pipe,
        .handle_result = tc_dst_pipe_handle_result,
        .close_src = tc_close_src_pipe,
        .close_dst = tc_close_dst_file,
    };

    tc_run(&tc);
}

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        errx(EXIT_FAILURE, "Usage:\n  %s source_file dest_file_template", argv[0]);
    }

    test_regular_files(argv[1], argv[2]);
    test_in_pipe(argv[1], argv[2]);
    test_out_pipe(argv[1], argv[2]);
    test_both_pipe(argv[1], argv[2]);

    return 0;
}
