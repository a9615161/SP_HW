/*
 * convert.c : take a file in the form 
 *  word1
 *  multiline definition of word1
 *  stretching over several lines, 
 * followed by a blank line
 * word2....etc
 * convert into a file of fixed-length records
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "dict.h"
#define BIGLINE 512

int main(int argc, char **argv) {
	FILE *in;
	FILE *out;        /* defaults */
	char line[BIGLINE];
	
	/* If args are supplied, argv[1] is for input, argv[2] for output */
	if (argc==3) {
		if ((in =fopen(argv[1],"r")) == NULL){DIE(argv[1]);}
		if ((out =fopen(argv[2],"w")) == NULL){DIE(argv[2]);}	
	}
	else{
		printf("Usage: convert [input file] [output file].\n");
		return -1;
	}

	/* Main reading loop : read word first, then definition into dr */

	/* Loop through the whole file. */
	while (fgets(line,BIGLINE,in) !=NULL) {
		
		/* Create and fill in a new blank record.
		 * First get a word and put it in the word field, then get the definition
		 * and put it in the text field at the right offset.  Pad the unused chars
		 * in both fields with nulls.
		 */
		if(strcmp(line,"\n" ) == 0 )
			continue;
		
		Dictrec newRd ;
		memset(&newRd,0,sizeof(newRd));
		/* Read word and put in record.  Truncate at the end of the "word" field.
		 *
		 * Fill in code. */
        line[strcspn(line, "\n")] = '\0';
        strncpy(newRd.word, line, WORD - 1);
		//incase line > WORD,Truncate with NULL
		newRd.word[WORD-1] = '\0';
		/* Read definition, line by line, and put in record.
		 *
		 * Fill in code. */
		//words are seperated by a null line
    	newRd.text[0] = '\0';  // 清空
    	while (fgets(line, BIGLINE, in) != NULL) {
    	    if (strcmp(line, "\n") == 0)
    	        break;  // 讀到空行 → definition 結束
    	    if (strlen(newRd.text) + strlen(line) >= TEXT - 1)
    	        break;  // 防止太長溢出
    	    strcat(newRd.text, line);
    	}
		/* Write record out to file.
		 *
		 * Fill in code. */
 		fwrite(&newRd, sizeof(Dictrec), 1, out);

	}

	fclose(in);
	fclose(out);
	return 0;
}
