#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define LAB11DEBUG 0

#define log_info(M, ...) fprintf(stderr, "\n\033[5m\033[31m[INFO]\033[0m (%s:%d) " M "\n", __FILE__, __LINE__)

char *hex_Number_l;
char *target__l;
const char *fpath__l;

int compare_l(char *t, char *buffer, int c){
    int i = 0;
    while (i < c){
        if (t[i] != buffer[i]){
            return 0; // Если не совпадает
        }
        i++;
    }
    return 1; // Если совпадает
}


int string_in_hex(char* target_from_walk_tree,const char* path_from_walk_tree)
{
	target__l=target_from_walk_tree;
	fpath__l=path_from_walk_tree;
	
	#if LAB11DEBUG==1
	fprintf(stdout,"...For_string_target_in_hex(char* target_from_walk_tree,const char* path_from_walk_tree)");
	#endif
	
	
	FILE *f;
	char sym;
	long int pos;

	f=fopen(fpath__l,"r");
	if(f==NULL)
	{
		perror(fpath__l);
		return 1;
	}
	pos=ftell(f);
	if(pos==-1)
	{
		perror(fpath__l);
	}
	#if LAB11DEBUG==1
	else
	{
		fprintf(stdout,"\n\033[32m---> Entry of file! --->\033[0m\n%s\n",fpath__l);
		fprintf(stdout,"target__l\t%s\n",target__l);
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
	
	

	
	
	#if LAB11DEBUG==1
	fprintf(stdout,"\033[32m <--\033[0m");
	fprintf(stdout,"\nj=%d",j);
	#endif
	
	int len=strlen(target__l);
	int count=0;
	for(int i=0;i<len;i++)
	{
			unsigned char cyr_sym=target__l[i];
			cyr_sym=(int)cyr_sym;

			count++;
	}
	
	count=count*2;
	#if LAB11DEBUG==1
	fprintf(stdout,"\n\033[32mcount=\033[0m\t%d\n",count);
	#endif
	
	
	// Перевод таргета в латинские буквы
	int count2=0;
	hex_Number_l=(char*)malloc(sizeof(char)*count);
	for(int i=0;i<len;i++)
	{
			unsigned char cyr_sym=target__l[i];
			cyr_sym=(int)cyr_sym;

			while (cyr_sym > 0) 
			{
        			int r = cyr_sym % 16;
        			char c = (r < 10) ? ('0' + r) : ('a' + r - 10);
        			
        			hex_Number_l[count2]=c;
				count2++;
        			cyr_sym /= 16;
   			}
	}
	
	//	Сортировка
	for(int i=0;i<count2;i=i+2)
	{
		char swap1=hex_Number_l[i];
		char swap2=hex_Number_l[i+1];
		hex_Number_l[i]=swap2;
		hex_Number_l[i+1]=swap1;
	}
	
	#if LAB11DEBUG==1
	fprintf(stdout,"\033[32mtarget before search\033[0m\t");
	for(int i=0;i<count2;i++)
	{
		fprintf(stdout,"%c",hex_Number_l[i]);
			
	}
	fprintf(stdout,"\n");
	#endif
	
	int scores_to_need=0;
	if(count==j)
	{
		int a=0;
		while(a!=count)
		{
			if(arr__[a]==hex_Number_l[a])
			{
				scores_to_need++;
				a++;
				continue;
			}
			break;
		}
		if(scores_to_need==count)
		{
			free(hex_Number_l);
			hex_Number_l=0;
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
				if(arr__[a]==hex_Number_l[i])
				{
					scores_to_need++;
					#if LAB11DEBUG==2
					fprintf(stdout,"\033[32mfind sovpadenie\033[0m\tarr__[%d]=%c\thex_Number_l[%d]=%c\tscores_to_need=%d\n",a,arr__[a],i,hex_Number_l[i],scores_to_need);
					#endif
					break;
				}
				else
				{
					#if LAB11DEBUG==2
					fprintf(stdout,"arr__[%d]=%c\thex_Number_l[%d]=%c\tscores_to_need=%d\n",a,arr__[a],i,hex_Number_l[i],scores_to_need);
					#endif
					scores_to_need=0;
				}
			}
		}
		*/

		
		if (compare_l(hex_Number_l, arr__, j))
		{
			free(hex_Number_l);
			hex_Number_l=0;
			free(arr__);
			arr__=0;
			fclose(f);
			#if LAB11DEBUG==1
			fprintf(stdout,"\033[32m<--- Exit of file! <---\033[0m\n");
			#endif
			return 0;
	        }

		char tmp;
		int a=0;
		while (a<j)
		{
		  tmp = arr__[a];
		  // Сдвигаем буфер
		  int j = 0;
		  while (j < count - 1)
		  {  
		    arr__[j] = arr__[j + 1];
		    j++;  
	          }
		    arr__[j - 1] = tmp;
		    if (compare_l(hex_Number_l, arr__, j))
		    {
		      free(hex_Number_l);
		      hex_Number_l=0;
		      free(arr__);
		      arr__=0;
		      fclose(f);
		      #if LAB11DEBUG==1
		      fprintf(stdout,"\033[32m<--- Exit of file! <---\033[0m\n");
		      #endif
		      return 0;
		    }
		    a++;
		}

		
	}
	free(hex_Number_l);
	hex_Number_l=0;
	free(arr__);
	arr__=0;
	#if LAB11DEBUG==1
	fprintf(stdout,"\033[32m<--- Exit of file! <---\033[0m\n");
	#endif
	fclose(f);
	
	return 1;
}
