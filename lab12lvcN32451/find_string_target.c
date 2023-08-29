#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define LAB11DEBUG 0

#define log_info(M, ...) fprintf(stderr, "\n\033[5m\033[31m[INFO]\033[0m (%s:%d) " M "\n", __FILE__, __LINE__)

char *target__s;
const char *fpath__s;

int For_String(char* target_from_walk_tree,const char* path_from_walk_tree)
{
	target__s=target_from_walk_tree;
	fpath__s=path_from_walk_tree;
	
	#if LAB11DEBUG==1
	fprintf(stdout,"...For_2bytesSymbol_Cyrillic(char* target_from_walk_tree,const char* path_from_walk_tree)");
	#endif
	
	
	FILE *f;
	char sym;
	long int pos;

	f=fopen(fpath__s,"r");
	if(f==NULL)
	{
		perror(fpath__s);
		return 1;
	}
	pos=ftell(f);
	if(pos==-1)
	{
		perror(fpath__s);
	}
	#if LAB11DEBUG==1
	else
	{
		fprintf(stdout,"\n\033[32m---> Entry of file! --->\033[0m\n%s\n",fpath__s);
		fprintf(stdout,"target__s\t%s\n",target__s);
		fprintf(stdout,"pos=%ld\n",pos);
	}
	
	#endif
	
	fseek(f, 0L, SEEK_END);
	pos=ftell(f);
	
	#if LAB11DEBUG==1
	fprintf(stdout,"pos=%ld\n",pos);
	#endif
	
	char *arr__=(char*)malloc((pos+1)*sizeof(char));
	
	// Чистим файл от ' ','\t','\n'
	#if LAB11DEBUG==1
	fprintf(stdout,"\033[32mChar from file --> \033[0m");
	#endif
	fseek(f, 0, SEEK_SET);
	int j=0;
	do
	{
		sym=getc(f);
		//если символ означает конец файла
		if(sym==EOF)
		{
			break;
		}
		else
		{
			//если , то не заносим в массив и идем дальше
			if(sym==' '||sym=='\t'||sym=='\n')
			{
				continue;
			}
			// если просто 0, то добавляем, а если это сочетание 0х, то пропускаем
			else if (sym=='0')
			{
				if(fgetc(f)=='x')
				{
					continue;
				}
				else
				{
					arr__[j]=sym;
					#if LAB11DEBUG==1
					fprintf(stdout,"%c",sym);
					#endif				
					j++;
				}
			}
			else
			{
				arr__[j]=sym;
				#if LAB11DEBUG==1
				fprintf(stdout,"%c",sym);
				#endif
				j++;
			}
		}
	}while(sym!=EOF);
	
	int len=strlen(target__s);
	int count=0;
	for(int i=0;i<len;i++)
	{
			unsigned char cyr_sym=target__s[i];
			cyr_sym=(int)cyr_sym;

			count++;
	}
	
	count=count*2;
	#if LAB11DEBUG==1
	fprintf(stdout,"\n\033[32mcount=\033[0m\t%d\n",count);
	#endif
	
	int scores_to_need=0;
	if(count==j)
	{
		int a=0;
		while(a!=count)
		{
			if(arr__[a]==target__s[a])
			{
				scores_to_need++;
				a++;
				continue;
			}
			break;
		}
		if(scores_to_need==count)
		{
			free(arr__);
			arr__=0;
			fclose(f);
			#if LAB11DEBUG==1
			fprintf(stdout,"\033[32m<--- Exit of file! <---\033[0m\n");
			#endif
			return 0;
		}
		
	}
	else
	{	
		/*
		// перебор массива символов пока не будет достигнут последний
		int a=0;
		#if LAB11DEBUG==2
		fprintf(stdout,"\n\033[32mcount=\033[0m\t%d\n",count);
		#endif
		int scores_to_need=0;
		for(int a=0;a!=j;a++)
		{
			for(int i=0;i<count;i++)
			{
				if(arr__[a]==hex_Number_s[i])
				{
					scores_to_need++;
					#if LAB11DEBUG==2
					fprintf(stdout,"\033[32mfind sovpadenie\033[0m\tarr__[%d]=%c\thex_Number_s[%d]=%c\tscores_to_need=%d\n",a,arr__[a],i,hex_Number_s[i],scores_to_need);
					#endif
					break;
				}
				else
				{
					#if LAB11DEBUG==2
					fprintf(stdout,"arr__[%d]=%c\thex_Number_s[%d]=%c\tscores_to_need=%d\n",a,arr__[a],i,hex_Number_s[i],scores_to_need);
					#endif
					scores_to_need=0;
				}
			}
		}
		*/
		
		// ТУТ ОШИБКИ VALGRIND 
		char * substr;
		substr = strstr(arr__,target__s);
		if(substr!=NULL)
		{
			free(arr__);
			arr__=0;
			fclose(f);
			#if LAB11DEBUG==1
			fprintf(stdout,"\033[32m<--- Exit of file! <---\033[0m\n");
			#endif
			return 0;
		}	
		
		
	}

	free(arr__);
	arr__=0;
	#if LAB11DEBUG==1
	fprintf(stdout,"\033[32m<--- Exit of file! <---\033[0m\n");
	#endif
	fclose(f);
	
	return 1;
}
