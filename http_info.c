/* 
 	http_info display http server info

	james@ustc.edu.cn 2017.09.24
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN 4096

int debug;
char url_file[MAXLEN];
int wait_time = 1;
int output_sql = 0;
char sql_table_name[MAXLEN];
char server[MAXLEN];
char soft[MAXLEN];

int php, asp, java, hpprinter;

void addstr(char *buf, char *str)
{
	if (buf[0] == 0)
		strncpy(buf, str, MAXLEN);
	else {
		strncat(buf + strlen(buf), ",", MAXLEN - strlen(buf));
		strncat(buf + strlen(buf), str, MAXLEN - strlen(buf));
	}
}

void checkstring(char *buf)
{
	int a, b, len = strlen(buf);
	a = b = 0;
	while (b < len) {
		if ((buf[b] >= 'a' && buf[b] <= 'z') ||
		    (buf[b] >= 'A' && buf[b] <= 'Z') ||
		    (buf[b] >= '0' && buf[b] <= '9') ||
		    (buf[b] == '.') ||
		    (buf[b] == '_') ||
		    (buf[b] == '/') ||
		    (buf[b] == '(') ||
		    (buf[b] == ')') || (buf[b] == '<') || (buf[b] == '>') || (buf[b] == ' ') || (buf[b] == ':') || (buf[b] == ',') || (buf[b] == ';')) {
			buf[a] = buf[b];
			a++;
		} else {
			if (debug)
				printf("DBG: skip char %c\n", buf[b]);
		}

		b++;
	}
}

void http_info_output(char *url, char *server, char *soft, char *tag)
{
	if (output_sql == 0) {
		printf("\"%s\" \"%s\" \"%s\" \"%s\"\n", url, server, soft, tag);
		fflush(NULL);
		return;
	}
	checkstring(url);
	checkstring(server);
	checkstring(soft);
	checkstring(tag);
	printf("replace into %s (url,server,soft,tag) values(\"%s\",\"%s\",\"%s\",\"%s\");\n", sql_table_name, url, server, soft, tag);
	fflush(NULL);
}

void http_info(char *url)
{
	FILE *fp;
	char buf[MAXLEN];
	char *p;
	int len;
	int got_res = 0;
	server[0] = 0;
	soft[0] = 0;
	php = asp = java = hpprinter = 0;
	snprintf(buf, MAXLEN, "curl -k --head -m %d %s 2>/dev/null", wait_time, url);
	fp = popen(buf, "r");
	if (fp == NULL) {
		fprintf(stderr, "popen %s error\n", buf);
		return;
	}
	while (fgets(buf, MAXLEN, fp)) {
		got_res = 1;
		len = strlen(buf);
		if (len <= 2)
			continue;
		while (1) {
			if (buf[len - 1] == '\r')
				len--;
			else if (buf[len - 1] == '\n')
				len--;
			else
				break;
		}
		buf[len] = 0;
		if (debug)
			printf("DBG: %s\n", buf);
		if (memcmp(buf, "Server: ", 8) == 0) {
			p = buf + 8;
			if (debug)
				printf("DBG: find server: %s\n", p);
			if (strcmp(p, "     ") == 0)
				continue;
			addstr(server, p);
			if (strstr(p, "Virata-EmWeb") != 0)
				hpprinter = 1;
			if (strstr(p, "Coyote") != 0)
				java = 1;
		}
		if (memcmp(buf, "Set-Cookie: ", 12) == 0) {
			p = buf + 12;
			if (debug)
				printf("DBG: find cookie:%s\n", p);
			if (strstr(p, "PHP"))
				php = 1;
			else if (strstr(p, "ASP"))
				asp = 1;
			if (strstr(p, "JSESSION"))
				java = 1;
		}
		if (memcmp(buf, "X-Powered-By: ", 14) == 0) {
			p = buf + 14;
			if (debug)
				printf("DBG: find soft:%s\n", p);
			addstr(soft, p);
			if (strstr(p, "PHP"))
				php = 1;
			else if (strstr(p, "ASP"))
				asp = 1;
		}
	}

	if (got_res == 0) {
		strcpy(buf, "FAIL");
		http_info_output(url, buf, "", "");
		return;
	}
	buf[0] = 0;
	if (php == 1)
		addstr(buf, "php");
	if (asp == 1)
		addstr(buf, "asp");
	if (java == 1)
		addstr(buf, "java");
	if (hpprinter == 1)
		addstr(buf, "hpprinter");
	http_info_output(url, server, soft, buf);
}

void http_info_url_file(char *url_file)
{
	FILE *fp;
	char buf[MAXLEN];
	fp = fopen(url_file, "r");
	if (fp == NULL) {
		printf("error open file %s\n", url_file);
		exit(-1);
	}
	while (fgets(buf, MAXLEN, fp)) {
		int len = strlen(buf);
		if (len <= 1)
			continue;
		while (1) {
			if (buf[len - 1] == '\r')
				len--;
			else if (buf[len - 1] == '\n')
				len--;
			else
				break;
		}
		buf[len] = 0;
		http_info(buf);
	}
	fclose(fp);
	exit(0);
}

void usage(void)
{
	fprintf(stderr, "%s\n",
		"usage: \n"
		"    http_info [ -d ] [ -w wait_time ] [ -s -t table_name ] [ -i url_file | url ]\n"
		"          -d              debug\n"
		"          -w wait_time    wait_time when do curl\n"
		"          -s              ouput sql replace into statement\n"
		"          -t table_name   sql replace table_name\n" "          -i url_file     read url from url_file\n");
}

int main(int argc, char *argv[])
{
	int c;
	while ((c = getopt(argc, argv, "dw:i:st:")) != EOF)
		switch (c) {
		case 'd':
			debug = 1;
			break;
		case 'w':
			wait_time = atoi(optarg);
			break;
		case 'i':
			strncpy(url_file, optarg, MAXLEN);
			break;
		case 's':
			output_sql = 1;
			break;
		case 't':
			strncpy(sql_table_name, optarg, MAXLEN);
			break;
		}
	if (sql_table_name[0] == 0)
		strcpy(sql_table_name, "http_info");
	if (url_file[0])
		http_info_url_file(url_file);
	else if (optind == argc - 1) {
		http_info(argv[optind]);
		exit(0);
	} else
		usage();
	exit(-1);
}
