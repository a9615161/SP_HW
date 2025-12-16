/*
 * lookup5 : local file ; setup a memory map of the file
 *           Use bsearch. We assume that the words are already
 *           in dictionary order in a file of fixed-size records
 *           of type Dictrec
 *           The name of the file is what is passed as resource
 */

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include "dict.h"

/*
 * This obscure looking function will be useful with bsearch
 * It compares a word with the word part of a Dictrec
 */

int dict_cmp(const void *a,const void *b) {
    return strcmp((char *)a,((Dictrec *)b)->word);
}

int lookup(Dictrec * sought, const char * resource) {
	static Dictrec * table;
	static int numrec;
	Dictrec * found;
	static int first_time = 1;

	if (first_time) {  /* table ends up pointing at mmap of file */
		int fd;
		long filsiz;

		first_time = 0;

		/* Open the dictionary.
		 * Fill in code. */
		if ((fd = open(resource, O_RDONLY)) < 0) {
            return UNAVAIL;
        }
		/* Get record count for building the tree. */
		filsiz = lseek(fd,0L,SEEK_END);
		numrec = filsiz / sizeof(Dictrec);

		/* mmap the data.
		 * Fill in code. */
		table = (Dictrec *)mmap(NULL, filsiz, PROT_READ, MAP_SHARED, fd, 0);
		close(fd);
	}

	/* search table using bsearch
	 * Fill in code. */
	found = NULL;
	// 參數1 (key): sought->word (我們只拿單字去比)
    // 參數2 (base): table (記憶體映射的起點)
    // 參數3 (nmemb): numrec (總共有幾個單字)
    // 參數4 (size): sizeof(Dictrec) (每個單字結構多大)
    // 參數5 (compar): dict_cmp (上面定義好的比較器)
	found = (Dictrec*)bsearch(sought->word,table,numrec,sizeof(Dictrec),dict_cmp);
    /*for(int i = 0;i<numrec;++i){
		if(dict_cmp(mapped[i].word,))
	}*/
	if (found) {
		strcpy(sought->text,found->text);
		return FOUND;
	}

	return NOTFOUND;
}
