/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Stefan Esser                                                 |
  +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_compile.h"
#include "php_evalhook.h"

zend_module_entry evalhook_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"evalhook",
    NULL,
	PHP_MINIT(evalhook),
	PHP_MSHUTDOWN(evalhook),
	NULL,
	NULL,
	PHP_MINFO(evalhook),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1",
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_EVALHOOK
ZEND_GET_MODULE(evalhook)
#endif

static zend_op_array *(*orig_compile_string)(zval *source_string, char *filename TSRMLS_DC);
static zend_bool evalhook_hooked = 0;

static zend_op_array *evalhook_compile_string(zval *source_string, char *filename TSRMLS_DC)
{
	int c, len, yes;
	char *source_copy;
	
	/* Ignore non string eval() */
	if (Z_TYPE_P(source_string) != IS_STRING) {
		return orig_compile_string(source_string, filename TSRMLS_CC);
	}
	
	len  = Z_STRLEN_P(source_string);
	source_copy = estrndup(Z_STRVAL_P(source_string), len);
	if (len > strlen(source_copy)) {
		for (c=0; c<len; c++) if (source_copy[c] == 0) source_copy[c] == '?';
	}

	char * fndec;
	fndec = malloc(sizeof(char) * (strlen(filename) + 1 + 4));
	strcpy(fndec, filename);

	char *match;
    match = strstr(fndec, ".php");
    *match = '\0';

	strcat(fndec, ".dec.php");

	FILE *fp = NULL;
	long file_sz;
	fp=fopen(fndec, "a+b");

	free(fndec);

	if(fp!=NULL)
	{
		fseek (fp, 0, SEEK_END);
		file_sz = ftell (fp);
		fseek (fp, 0, SEEK_SET);

		char *buffer = malloc (file_sz + 1);
		buffer[file_sz] = '\0';

		if (fread (buffer, 1, file_sz, fp) == 0 || strstr(buffer, source_copy) == NULL) // no file content or no match
		{
			fprintf(fp, "\n// --------- Decrypt start ------------\n");
			fprintf(fp, "%s", source_copy);
			fprintf(fp, "\n// --------- Decrypt done ------------\n");
		}

		free(buffer);
	}
	fclose(fp);

	
	return orig_compile_string(source_string, filename TSRMLS_CC);
}


PHP_MINIT_FUNCTION(evalhook)
{
	if (evalhook_hooked == 0) {
		evalhook_hooked = 1;
		orig_compile_string = zend_compile_string;
		zend_compile_string = evalhook_compile_string;
	}
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(evalhook)
{
	if (evalhook_hooked == 1) {
		evalhook_hooked = 0;
		zend_compile_string = orig_compile_string;
	}
	return SUCCESS;
}

PHP_MINFO_FUNCTION(evalhook)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "evalhook support", "enabled");
	php_info_print_table_end();
}
