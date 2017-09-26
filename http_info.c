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
int wait_time = 2;
int output_sql = 0;
char sql_table_name[MAXLEN];
char server[MAXLEN];
char soft[MAXLEN];

int php, asp, java, hpprinter, h3c, lenovo;
int netgear, zhonshipcam, hkvsipcam, dlink, mssql, qnap, ipmi, pdyq, labview;
int vmware;

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
	if (buf[0] == 0)
		return;
	a = b = 0;
	while (b < len) {
		if ((buf[b] >= 'a' && buf[b] <= 'z') ||
		    (buf[b] >= 'A' && buf[b] <= 'Z') ||
		    (buf[b] >= '0' && buf[b] <= '9') ||
		    (buf[b] == '.') ||
		    (buf[b] == '_') ||
		    (buf[b] == '-') ||
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
	buf[a] = 0;
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
	printf("replace into %s (url,server,soft,tag,lastcheck) values(\"%s\",\"%s\",\"%s\",\"%s\",now());\n", sql_table_name, url, server, soft, tag);
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
	php = asp = java = hpprinter = h3c = lenovo = 0;
	netgear = zhonshipcam = hkvsipcam = dlink = mssql = qnap = ipmi = pdyq = labview = vmware = 0;
	if (debug)
		printf("DBG: url=%s\n", url);
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
			else if (strcmp(p, "Mrvl-R1_0") == 0)
				hpprinter = 1;
			else if (strcmp(p, "Mrvl-R2_0") == 0)
				hpprinter = 1;
			else if (strcmp(p, "HP_Compact_Server") == 0)
				hpprinter = 1;
			else if (memcmp(p, "HP HTTP Server;", 15) == 0)
				hpprinter = 1;
			else if (strstr(p, "Coyote") != 0)
				java = 1;
			else if (strcmp(p, "WMI V5") == 0)
				h3c = 1;
			else if (strcmp(p, "Switch") == 0)
				h3c = 1;
			else if (strcmp(p, "uhttpd/1.0.0") == 0) {
				if (memcmp(url, "https://", 8) == 0)
					netgear = 1;
			} else if (strcmp(p, "thttpd/2.27 19Oct2015") == 0)
				zhonshipcam = 1;
			else if (strcmp(p, "Hikvision-Webs") == 0)
				hkvsipcam = 1;
			else if (strcmp(p, "DNVRS-Webs") == 0)
				hkvsipcam = 1;
			else if (strcmp(p, "App-webs/") == 0)
				hkvsipcam = 1;
			else if (strcmp(p, "mini_httpd/1.19 19dec2003") == 0)
				dlink = 1;
			else if (strcmp(p, "Microsoft-HTTPAPI/2.0") == 0)
				mssql = 1;
			else if (strcmp(p, "Mbedthis-Appweb/2.4.2") == 0)
				lenovo = 1;
			else if (strcmp(p, "http server 1.0") == 0)
				qnap = 1;
			else if (strcmp(p, "GoAhead-Webs") == 0)
				ipmi = 1;
			else if (strcmp(p, "GoAhead-Webs/2.5.0") == 0)
				pdyq = 1;
			else if (strcmp(p, "Embedthis-http") == 0)
				labview = 1;
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
	pclose(fp);

	if (got_res == 0) {
		strcpy(buf, "FAIL");
		http_info_output(url, buf, "", "");
		return;
	}

	if ((server[0] == 0) && (soft[0] == 0) && (memcmp(url, "https://", 8) == 0)) {	// https, nothing got, try get ca information
		FILE *pfp;
		char sslurl[MAXLEN];
		snprintf(sslurl, MAXLEN, "%s", url + 8);
		p = sslurl;
		while (*p && (*p != '/'))
			p++;
		*p = 0;

		if (strchr(sslurl, ':'))
			snprintf(buf, MAXLEN, "timeout 1 openssl s_client -connect %s < /dev/null 2>&1", sslurl);
		else
			snprintf(buf, MAXLEN, "timeout 1 openssl s_client -connect %s:443 < /dev/null 2>&1", sslurl);
		if (debug)
			printf("DBG: try %s\n", buf);
		pfp = popen(buf, "r");
		if (pfp && fgets(buf, MAXLEN, pfp)) {	// get first line
			if (strstr(buf, "@vmware.com"))
				vmware = 1;
			else if (strstr(buf, "VMware"))
				vmware = 1;
			else if (strstr(buf, "@ami.com"))
				ipmi = 1;
			else if (strstr(buf, "Super Micro Computer"))
				ipmi = 1;
			else if (strstr(buf, "IPMI"))
				ipmi = 1;
			else if (strstr(buf, "iDRAC"))
				ipmi = 1;
			else if (strstr(buf, "HP Jetdirect"))
				hpprinter = 1;
			else if (debug)
				printf("DBG: unknow ca %s", buf);
			pclose(pfp);
		}
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
	if (h3c == 1)
		addstr(buf, "h3c");
	if (lenovo == 1)
		addstr(buf, "lenovo");
	if (netgear == 1)
		addstr(buf, "netgeare");
	if (zhonshipcam == 1)
		addstr(buf, "zhonshipcam");
	if (hkvsipcam == 1)
		addstr(buf, "hkvsipcam");
	if (dlink == 1)
		addstr(buf, "dlink");
	if (mssql == 1)
		addstr(buf, "mssql");
	if (qnap == 1)
		addstr(buf, "qnap");
	if (ipmi == 1)
		addstr(buf, "ipmi");
	if (pdyq == 1)
		addstr(buf, "pdyq");
	if (labview == 1)
		addstr(buf, "labview");
	if (vmware == 1)
		addstr(buf, "vmware");
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
