#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*

All integeres in systems where <sizeof(int) == 32> will fit in 12 bytes when converted to string.
Therefore we define that as <INT_SIZ>.

*/

#ifndef INT_SIZ
#define INT_SIZ 12		
#endif


struct entry 
{

	int uid; /* user id (positive integer) */
	int access_type; /* access type values [0-2] */
	int action_denied; /* is action denied values [0-1] */
	int file_exists; /* file exists values [0-1] */

	char *date; /* file access date */
	char *time; /* file access time */

	char *file; /* filename (string) */
	char *fingerprint; /* file fingerprint */

	/* add here other fields if necessary */
	/* ... */
	/* ... */

	struct entry *next; //pointer on next object.


};







void printlist(struct entry *head);
struct entry *read_log(FILE *fp);
struct entry *push_log(struct entry *head, int uid, char *filepath, int file_exists,char *date, char *time, int action_type, int action_status, char *file_hash);
int get_lines(FILE *fp);
struct tm *get_time();
void list_users(struct entry *log, int threshold);
void list_mods(struct entry *log, char *target_file);
void print_log(struct entry *log);
void list_files(struct entry *log, int time_range, int threshold);
void list_files_created(FILE *log, int t);




void usage(void)
{
	printf(
	       "\n"
	       "usage:\n"
	       "\t./acmon \n"
		   "Options:\n"
		   "-m, Prints malicious users\n"
		   "-i <filename>, Prints table of users that modified "
		   "the file <filename> and the number of modifications\n"
		   "-v <threshold>, Prints files created between."
		   "-h, Help message\n\n"
		   );

	exit(1);
}


void list_unauthorized_accesses(FILE *log)
{

	int c = get_lines(log);

	//init. a list of entries.
	struct entry *head = read_log(log);
	


	//list malicious entries, zero tolerance.

	list_users(head, 1); 


	
	return;

}


void list_file_modifications(FILE *log, char *file_to_scan)
{

	int c = get_lines(log);

	//init. a list of entries.
	struct entry *head = read_log(log);
	list_mods(head, file_to_scan);
	

	return;

}


void list_files_created(FILE *log, int t)
{
	struct entry *head = read_log(log);
	list_files(head, 20, t);

}


int  main(int argc, char *argv[])
{

	int ch;
	FILE *log;

	if (argc < 2)
		usage();

	//modify this if not testing..
	log = fopen("./test.log", "r");
	if (log == NULL) {
		printf("Error opening log file \"%s\"\n", "./log");
		return 1;
	}

	while ((ch = getopt(argc, argv, "hi:hv:m")) != -1) {
		switch (ch) {		
		case 'i':
			list_file_modifications(log, optarg);
			break;
		case 'm':
			list_unauthorized_accesses(log);
			break;
		case 'v':
			list_files_created(log, atoi(optarg));
			break;
		default:
			usage();
		}

	}


	/* add your code here */
	/* ... */
	/* ... */
	/* ... */
	/* ... */


	fclose(log);
	argc -= optind;
	argv += optind;	
	
	return 0;
}



void printlist(struct entry* head)
{
	struct entry *curr = head;

	
	while(curr != NULL)
	{
		
		printf("%d %s %d\n",curr->uid, curr->file, curr->action_denied);
		curr = curr->next;
	}
}

struct entry *read_log(FILE *fp)
{

	struct entry *head = NULL;
	size_t line_size;
	char *line = NULL;

	//get the first of them.
    line_size = getline(&line, &line_size, fp);




	//read <log_lines> via readline.
	while(line_size > 1)
	{
		
		//a structure to save tokens
		char *str[9];

		//tokenize with " " as delimiter.
		char *token = strtok(line, " ");
		int i=0;
		
		while(token != NULL)
		{
			str[i] = token;
			
			//tokenize again
			token = strtok(NULL, " ");
			i++;
		}

		//push on a single linked list.
		head = push_log(head, atoi(str[1]), str[2], atoi(str[3]), str[4], str[5], atoi(str[6]), atoi(str[7]), str[8]);

		
		//read again.
		line_size = getline(&line, &line_size, fp);

	}

	return head;
	
	
	


}
/*
	Creates nodes of <struct entry> and then pushed into the right position in the given <struct entry *> list.
	
	Args:

			-arg1: 		head of the log list.
			-arg2-9: 	values for the newly created node.
	Returns:

			Always the head of the list.
	


*/


struct entry *push_log(struct entry *head, int uid, char *filepath, int file_exists,char *date, char *time, int action_type, int action_status, char *file_hash)
{


	struct entry *node = (struct entry *)malloc(sizeof(struct entry));
	struct entry *curr = head;


	//init. content

	node->uid = uid;
	node->file = malloc(strlen(filepath));
	strcpy(node->file, filepath);
	node->file_exists = file_exists;
	node->date = malloc(strlen(date));
	strcpy(node->date, date );
	node->time = malloc(strlen(time));
	strcpy(node->time, time);
	node->access_type = action_type;
	node->action_denied = action_status;
	node->fingerprint = malloc(strlen(file_hash));
	strcpy(node->fingerprint, file_hash);	
    node->next = NULL;

	//one element list.
	if(head == NULL)
		return node;


	//seek the end of the list.
	while(curr->next != NULL)
		curr = curr->next;


	//append node.
	curr->next = node;




    return head;




}



/*

	A utility function to count the lines of a file, specified by <FILE *fp>.

*/

int get_lines(FILE *fp)
{

	char curr;
	unsigned int count = 0;
	while((curr = fgetc(fp)) != EOF) 
	{
		
	  	if(curr == '\n')
	    	count++;

    }

   //set pointer to start.
   rewind(fp);


   return count;

}

/*

	Lists any user attempted more than <int threshold attempts.>

*/

void list_users(struct entry *log, int threshold)
{
	struct entry *prev = log;
	int count;
	int malicious[0xFFF]; //that should be enough.
	int m_count = 0;
	
	
	while(prev)
	{

		count = 0;
		struct entry *curr = prev;
		while(curr)
		{
			if((curr->uid == prev->uid ) && (curr->action_denied == 1))
			{
				
				count++;
				
			}

			curr = curr->next;
		}

		//prompt user if actual occurence.
		if(count >= threshold)
		{

			int in_list = 0;
			for(int i = 0; i < m_count; i++)
			{
				if(malicious[i] == prev->uid)
					in_list = -1;
			}
			if(in_list == 0)
			{
				malicious[m_count] = prev->uid;
				m_count++;
				
			}
			
				
		}

		
		//just move.
		prev = prev->next;
	}
		
	//prompt before exiting..
	if(!m_count)
		fprintf(stdout,"No malicious users in system..\n");
	else
	{
		fprintf(stdout,"Malicious users:\n");
		for(int i = 0; i < m_count; i++)
		{
			fprintf(stdout, "[uid]:\t%d\n", malicious[i]);
		}
	}

}


/*

	Lists the user who modified a file specified by <char * target_file> and counts accesses.
	
	Notes:

		- This should be Theta(n^2) thus is pretty straight-forward and quick in C.



*/


void list_mods(struct entry *log, char *target_file)
{
	struct entry *curr = log;
	int count;
	char *init_hash = NULL;
	int modificants[0xFFF]; //that should be enough.
	int attempts[0xFFF];
	int m_count = 0;
	
	//find the first file occurence in log.
	while((curr != NULL ) && (strcmp(target_file, curr->file) != 0))
	{
		curr = curr->next;
	}
	
	//no hit,exiting.
	if(!curr)
	{
		printf("No such file exists..\n");
		return;
	}

	//remember the initial hash.
	init_hash = curr->fingerprint;
	
		
	//start from where u are, if seen again the file might be modified.
	while(curr)
	{
		//if we're talking about the same file and the file is acutally altered/
		if((strcmp(init_hash, curr->fingerprint) != 0) && strcmp(curr->file, target_file) == 0)
		{

			//existance flag.
			int in_list = 0;

			//this increases complexity but works well..
			for(int i = 0; i < m_count; i++)
			{
				//if already there increase access count, woorah we got a bad boy here.
				if(modificants[i] == curr->uid)
				{
					attempts[i] += 1;
					in_list  = 1;
				}

			}
			//first occurence, we should record this bad boy..
			if(in_list == 0)
			{
				modificants[m_count] = curr->uid;
				attempts[m_count] = 1;
				m_count++;
			}

		
		}

		//move on next log.
		curr = curr->next;
			
	}	
				
			
	//prompt before exiting..
	if(!m_count)
		fprintf(stdout,"No modifications found..\n");
	else
	{
		fprintf(stdout,"Users who modified \"%s\":\n\n", target_file);
		fprintf(stdout, "[uid]\t [times modified]\t\n\n");
		for(int i = 0; i < m_count; i++)
		{
			fprintf(stdout, "%-5d\t%10d\n", modificants[i], attempts[i]);
		}
	}



}

/*

	Creates a list of files created in the time range of [start_time - threshold, start_time]

	Arguments:

				-arg1: a pointer to head of the log list.
				-arg2: the time range to search between, should be in minutes [0, 60].
				-arg3: a threshold to define minimum file number.


*/

void list_files(struct entry *log, int time_range, int threshold)
{

	unsigned int files_created = 0;

	//cache the head pointer.
	struct entry *curr = log;
	
	//get the time and format it in respect with list contents.
	struct tm *t = get_time();



	//this is actually pre-compile-time allocation, anyway..
	char *init_time = (char *)malloc(3*INT_SIZ);  // #|hour, minute, sec| = 3 elements
	char *curr_date = (char *)malloc(3*INT_SIZ);  // #|year, month, dat| = 3 elements

	//adjust initial time.
	t->tm_min =(t->tm_min - time_range)%60;

	//adjusting values in time range.
	if(t->tm_min < 0)
	{
		t->tm_min += 60;
		t->tm_hour -= 1;
	}
	

	//converting to string, like our records are [definetlly easier than doing the opossite], format should be the exact same as on logger.c for the <strcmp> to work later on..
	sprintf(init_time, "%d:%d:%d", t->tm_hour, t->tm_min, t->tm_sec); 
	sprintf(curr_date, "%d-%d-%d", t->tm_year, t->tm_mon, t->tm_mday);
	

	//itterate through the log list.
	while(curr)
	{
		
		//if talking about the same date and specified time range, do stuff..
		if((strcmp(curr_date, curr->date) == 0) && (strcmp(init_time, curr->time) < 0))
		{
			files_created++;
		}


		//move on.
		curr = curr->next;
	}



	//prompt user the result.

	fprintf(stdout, "[Info]: %d files were created in the last %d minutes.\n", files_created, time_range);

	
	//free resources, although not actually needed the way memory is allocated.
	free(init_time);
	free(curr_date);


}




void print_log(struct entry *log)
{
	fprintf(stdout, "[Log]: [uid]: %-5d [filename]: %s [File Exists (0/1)]: %d [Date]: %s [Time]:%s\n", log->uid, log->file, log->file_exists, log->date, log->time);
}



/*

	Returns system time when called.

	Note:

		 - When in Ubuntu tm_year + 1900 will produce the proper year.
		 - Function handles that field change as well.

*/

struct tm *get_time()
{

	struct tm *retVal;
	time_t rawtime;

	//get time, adjust on local settings.
	time(&rawtime);
	retVal = localtime(&rawtime);

	//fix calibration issue by adding 1900 to the result.
	retVal->tm_year += 1900;
	retVal->tm_mon++;


	return retVal;
}