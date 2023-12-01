#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/wait.h>

#include <mysql.h>
#include <mysql/mysqld_error.h>

int begin(MYSQL *sql)
{
#if 1
	if (mysql_query(sql, "START TRANSACTION;")) {
		warnx("db START failed: %s", mysql_error(sql));
		return -1;
	}
#else
	if (mysql_query(&mysql, "LOCK TABLES sources WRITE, structs WRITE, members WRITE, uses WRITE;")) {
		warnx("db LOCK failed: ", mysql_error(&mysql) << "\n";
		return -1;
	}
#endif
	return 0;
}

int end(MYSQL *sql)
{
#if 1
	if (mysql_commit(sql)) {
		warnx("db commit failed: %s", mysql_error(sql));
		return -1;
	}
#else
	if (mysql_query(&mysql, "UNLOCK TABLES;")) {
		warnx("db UNLOCK failed: ", mysql_error(&mysql) << "\n";
		return -1;
	}
#endif
	return 0;
}

static int connect(MYSQL *mysql)
{
	if (!mysql_init(mysql)) {
		warnx("mysql_init\n");
		return -1;
	}

	if (!mysql_real_connect(mysql, "127.0.0.1", "xslaby", NULL, "structs_66", 0, NULL, 0)) {
		warnx("mysql_real_connect: %s", mysql_error(mysql));
		goto fail;
	}

	return 0;
fail:
	mysql_close(mysql);
	return -1;
}

 __attribute__((noreturn)) void child()
{
	MYSQL mysql;

	if (connect(&mysql))
		exit(1);

	if (mysql_autocommit(&mysql, 0)) {
		warnx("mysql_autocommit: %s", mysql_error(&mysql));
		goto fail;
	}

	MYSQL_STMT *insSrc = mysql_stmt_init(&mysql);
	if (!insSrc) {
		warnx("db stmt_init failed: %s", mysql_error(&mysql));
		goto fail;
	}

	const char create[] = "INSERT IGNORE INTO sources(src) VALUES (?)";
	if (mysql_stmt_prepare(insSrc, create, strlen(create))) {
		warnx("db prep failed for: %s: %s", create, mysql_error(&mysql));
		goto fail;
	}

	pid_t pid = getpid();

	if (begin(&mysql) < 0)
		exit(1);

	for (unsigned a = 0; a < 50; a++) {
		char src[24];
		unsigned long len = snprintf(src, sizeof(src), "src-%.6u-%.5u", pid, a);

		MYSQL_BIND b = {
			.length = &len,
			.buffer = src,
			.buffer_length = sizeof(src),
			.buffer_type = MYSQL_TYPE_STRING,
		};

		if (mysql_stmt_bind_param(insSrc, &b)) {
			warnx("db bind failed: %s", mysql_stmt_error(insSrc));
			exit(1);
		}

		while (mysql_stmt_execute(insSrc)) {
			switch (mysql_stmt_errno(insSrc)) {
			case ER_LOCK_DEADLOCK:
			case ER_LOCK_WAIT_TIMEOUT:
				usleep(1000);
				continue;
			}

			warnx("[%u] db exec failed: %s", pid, mysql_stmt_error(insSrc));
			goto fail_stmt;
		}

		if (mysql_stmt_affected_rows(insSrc) != 1)
			warnx("%s: %llu", src, mysql_stmt_affected_rows(insSrc));
	}

	if (end(&mysql) < 0)
		exit(1);

fail_stmt:
	mysql_stmt_close(insSrc);
fail:
	mysql_close(&mysql);
	exit(0);
}

#define CHILDREN 10

int main()
{
	MYSQL mysql;

	if (connect(&mysql))
		return EXIT_FAILURE;

	if (mysql_query(&mysql, "DELETE FROM sources;")) {
		warnx("cannot DELETE: %s", mysql_error(&mysql));
		mysql_close(&mysql);
		return EXIT_FAILURE;
	}

	mysql_close(&mysql);

	for (unsigned i = 0; i < CHILDREN; i++) {
		switch (fork()) {
		case -1:
			i = CHILDREN;
			break;
		case 0:
			child();
		default:
			break;
		}
	}

	for (unsigned i = 0; i < CHILDREN; i++)
		wait(NULL);

	return 0;
}
