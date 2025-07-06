
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define STAGE_NUM_ONE 1							/* Stage numbers */
#define STAGE_NUM_TWO 2
#define STAGE_NUM_THREE 3
#define STAGE_NUM_FOUR 4
#define STAGE_HEADER "Stage %d\n==========\n"	/* Stage header format string */

#define VOCABULARY_SIZE 50						/* Number of input words */
#define MAX_LETTERS 21                          /* Include nul byte */
#define MAX_SENTENCE 12							/* Including start and end */

typedef char word_t[MAX_LETTERS];
typedef double prob_t;

typedef struct word_rec_t word_record_t;

typedef struct {
	/* Add your word record struct definition */
	// Each word record contains the word and its probability
	word_t word;
	prob_t prob;
	word_t next_word;
	int next_word_index;
} word_rec_t;

typedef char data_t[MAX_LETTERS];								/* To be modified for Stage 3 */


typedef struct{													/* Stage 4 */
	char sentence[MAX_SENTENCE][MAX_LETTERS];
	int last;
	prob_t sentence_prob;
	int num_words;
} sent_t ;

typedef struct node node_t;

struct node {
	data_t data;
	node_t *next;
};

typedef struct {
	node_t *head;
	node_t *foot;
} list_t;

/****************************************************************/

/* Function prototypes */
list_t *make_empty_list(void);
void free_list(list_t *list);
list_t *insert_at_foot(list_t *list, data_t value);

void print_stage_header(int stage_num);

void stage_one(word_rec_t word_record[], int *n);
void stage_two(word_rec_t word_record[], int n, double trans_probability[][n]);
void stage_three(word_rec_t word_rec[], int n);
void stage_four(word_rec_t word_record[], int n, double trans_probability[][n]);

// Stage 1
int descending_prob(const void *word1, const void *word2);

// Stage 3
void print_list(list_t *word_list);

// Stage 4
void expand_partial_sentence(int n, int *expansion_count, double trans_probability[][n],
							 word_rec_t word_record[], sent_t *sents, sent_t *new_sents);
void best_sentence(sent_t *new_sents, int expansion_count, int *index_1, int *index_2);
int descending_sents(const void *first, const void *second);
void save_top_sents(sent_t *sents, sent_t *new_sents, int index_1, int index_2);

/****************************************************************/

/* Main function controls all the action; modify if needed */
int
main(int argc, char *argv[]) {
	/* Add variables to hold the input data */
	word_rec_t word_record[VOCABULARY_SIZE];
	int n;
	/* Stage 1: read word records */
	stage_one(word_record, &n); 
	
	/* Stage 2: read transition probabilities */
	double trans_probability[n][n];
	stage_two(word_record, n, trans_probability);
	
	/* Stage 3: generate text with transition probabilities */
	stage_three(word_record, n);
	
	/* Stage 4: generate text with transition probabilities, advanced */
	stage_four(word_record, n, trans_probability);
	
	return 0;
}

/****************************************************************/

/* Stage 1: read word records */
void 
stage_one(word_rec_t word_record[], int *n) {
	/* Reads the data from the input files and generates a sentence based
	on the top-10 words identified by descending order of the probability (exclusive of <start>
	and <end>) */

	// Create an array of the struct type to store all words
	scanf("%d", n);
	word_rec_t sorted_word_record[*n - 2];

	char start[MAX_LETTERS], end[MAX_LETTERS];
	double start_prob, end_prob;
	scanf("%s %lf", end, &end_prob);
	scanf("%s %lf", start, &start_prob);

	strcpy(word_record[0].word, end);
	word_record[0].prob = end_prob;

	strcpy(word_record[1].word, start);
	word_record[1].prob = start_prob;

	// Store the words and their respective probabilities to the array
	for(int i = 2; i < *n; i++){
		scanf("%s %lf", word_record[i].word, &word_record[i].prob);
	}

	// Make a copy of the array for sorting
	for(int i = 0; i < *n - 2; i++){
		sorted_word_record[i] = word_record[i + 2];
	}
	qsort(sorted_word_record, *n - 2, sizeof(word_rec_t), descending_prob);

	/* Print stage header */
	print_stage_header(STAGE_NUM_ONE);

	// Print the sentence
	printf("%s ", word_record[1].word); // print <start>
	if (*n < 12){
		for(int i = 0; i < *n - 2; i++){
			printf("%s ", sorted_word_record[i].word);
		}
	} else{
		for(int i = 0; i < 10; i++){
			printf("%s ", sorted_word_record[i].word);
		}
	}
	
	printf("%s\n", word_record[0].word);
	printf("\n");
}

/* Stage 2: read transition probabilities */
void 
stage_two(word_rec_t word_record[], int n, double trans_probability[][n]) {
	/* Reads the transition probability matrix and determines the most likely next
	word for each word on the record based on the highest transition probabilities */

	// Read the transition matrix
	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			scanf("%lf", &trans_probability[i][j]);
		};
	}

	// Insert the transition probabilities into the struct
	for(int i = 1; i < n; i++){
		double max_probability = 0.0; // Iterate through each column in row i
		int next_index = 0;

		for(int j = 0; j < n; j++){
			if(trans_probability[i][j] > max_probability){
				max_probability = trans_probability[i][j];
				next_index = j;
			}
		}
		word_record[i].next_word_index = next_index;
		strcpy(word_record[i].next_word, word_record[next_index].word);
	}

	/* Print stage header */
	print_stage_header(STAGE_NUM_TWO);

	// Find the next word with highest probability
	for(int i = 1; i < n; i++){
		printf("%s -> %s\n", word_record[i].word, word_record[i].next_word);
	}

	printf("\n");
}

/* Stage 3: generate text with transition probabilities */
void 
stage_three(word_rec_t word_record[], int n) {
	/* Generates a linked list sentence by appending the most likely next word
	iteratively, stops until it ends with <end> or 10 words are reached */

	list_t *word_list = make_empty_list();
	insert_at_foot(word_list, word_record[1].word); // Insert <start>

	// Insert new word based on the transition probabilities
	int word_index = 1;
	for(int i = 0; i < 10; i++){
		if(word_index == 0){
			break;
		} else{
			insert_at_foot(word_list, word_record[word_index].next_word);
			word_index = word_record[word_index].next_word_index;
		}
	}
	if (word_index != 0){
		insert_at_foot(word_list, word_record[0].word);
	}
	
	/* Print stage header */
	print_stage_header(STAGE_NUM_THREE);

	// Print out the list
	print_list(word_list);
	free_list(word_list);
	printf("\n");
}

/* Stage 4: generate text with transition probabilities, advanced */
void stage_four(word_rec_t word_record[], int n, double trans_probability[][n]) {
	/* Constructs a sentence by expanding top-2 partial sentences iteratively,
	choosing the best 2 at each step until both end reaches <end> or 10 iterations */

	sent_t *sents = malloc(2 * sizeof(sent_t));
	sent_t *new_sents = malloc(2 * n * sizeof(sent_t));

	// At start sents only has 1 partial sentence
	strcpy(sents[0].sentence[0], word_record[1].word); // Insert <start>
	sents[0].last = 1;
	sents[0].sentence_prob = 1.0;
	sents[0].num_words = 1;
	
	for(int iter = 0; iter < 10; iter++){
		int expansion_count = 0;
		for(int i = 0; i < 2; i++){
			 // If the last word is <end>
			if(sents[i].last == 0){
				new_sents[expansion_count++] = sents[i];
			}else{
				// Otherwise, create new partial sentence
				expand_partial_sentence(n, &expansion_count, trans_probability, word_record,
				&sents[i], new_sents);
			}
		}
		// Choose two partial sentences in new_sents with highest probability
		int index_1, index_2;
		best_sentence(new_sents, expansion_count, &index_1, &index_2);
		
		// Copy the two partial sentences back to sents
		save_top_sents(sents, new_sents, index_1, index_2);
		
		// If both ends with end, stop
		if(sents[0].last == 0 && sents[1].last == 0){
			break;
		}

	}
	// If the chosen partial sentence does not end with <end>
	if(sents[0].last != 0 && sents[0].num_words < MAX_SENTENCE){
		strcpy(sents[0].sentence[sents[0].num_words], word_record[0].word);
		sents[0].num_words++;
	}
	/* Print stage header */
	print_stage_header(STAGE_NUM_FOUR);

	// Print the chosen partial sentence
	for(int word = 0; word < sents[0].num_words; word++){
		if(word == sents[0].num_words - 1){
			printf("%s", sents[0].sentence[word]);
		}else{
			printf("%s ", sents[0].sentence[word]);
		}
	}
	
	printf("\n");
	free(sents);
	free(new_sents);
}

/****************************************************************/

/* Stage 1 helper functions */
// Comparative function for qsort
int descending_prob(const void *word1, const void *word2){
	word_rec_t *word1_rec = (word_rec_t *)word1;
	word_rec_t *word2_rec = (word_rec_t *)word2;
	if (word1_rec->prob < word2_rec->prob){
		return 1;
	} else{
		return -1;
	}
}

/* Stage 3 helper functions */
// Prints the linked list
void print_list(list_t *word_list){
	node_t *current_node = word_list->head;
	while(current_node != NULL){
		if(current_node == word_list->head){
			printf("%s", current_node->data);
			current_node = current_node->next;
		} else{
			printf(" %s", current_node->data);
			current_node = current_node->next;
		}
	}
	printf("\n");
}

/* Stage 4 helper functions */
// Expands partial sentences, adds the next possible word with a non-zero transition probability
void expand_partial_sentence(int n, int *expansion_count,double trans_probability[][n],
word_rec_t word_record[], sent_t *sents, sent_t *new_sents){
	if(sents->last >= 0 && sents->last < n){
		for(int j = 0; j < n; j++){
			double trans_prob = trans_probability[sents->last][j];

			if(trans_prob > 0){
				sent_t temp;
				temp.sentence_prob = sents->sentence_prob * trans_prob;
				temp.last = j;
				temp.num_words = sents->num_words + 1;
				for(int word = 0; word < sents->num_words; word++){
					strcpy(temp.sentence[word], sents->sentence[word]);
				}
				strcpy(temp.sentence[temp.num_words - 1], word_record[j].word);
				new_sents[*expansion_count] = temp;
				(*expansion_count)++;
			}
		}
		
	}
}

// Finds the top-2 partial sentences in new_sents
void best_sentence(sent_t *new_sents, int expansion_count, int *index_1, int *index_2){
	*index_1 = -1;
	*index_2 = -1;

	qsort(new_sents, expansion_count, sizeof(sent_t), descending_sents);
	if(expansion_count > 0){
		*index_1 = 0;
	}
	if(expansion_count > 1){
		*index_2 = 1;
	}
}

// Compare function for qsort for sorting partial sentences
int descending_sents(const void *first, const void *second){
	const sent_t *first_sent = (const sent_t *)first;
	const sent_t *second_sent = (const sent_t *)second;
	if (second_sent->sentence_prob > first_sent->sentence_prob){
		return 1;
	} else if(second_sent->sentence_prob < first_sent->sentence_prob){
		return 0;
	} else{
		return -1;
	}
}

// Copies the top-2 partial sentences back to sents
void save_top_sents(sent_t *sents, sent_t *new_sents, int index_1, int index_2){
	sents[0] = new_sents[index_1];
	if(index_2 != -1){
		// If only one record in new_sents
		sents[1] = new_sents[index_2];
	}else{
		sents[1].last = 0;
		sents[1].num_words = 0;
		for(int clear = 0; clear < MAX_SENTENCE; clear++){
			sents[1].sentence[clear][0] = '\0';
		}
	}
}


/* Print stage header given stage number */
void 
print_stage_header(int stage_num) {
	printf(STAGE_HEADER, stage_num);
}

/****************************************************************/

/* Create an empty list */
list_t
*make_empty_list(void) {
	list_t *list;

	list = (list_t*)malloc(sizeof(*list));
	assert(list!=NULL);
	list->head = list->foot = NULL;

	return list;
}

/* Free the memory allocated for a list (and its nodes) */
void
free_list(list_t *list) {
	node_t *curr, *prev;

	assert(list!=NULL);
	curr = list->head;
	while (curr) {
		prev = curr;
		curr = curr->next;
		free(prev);
	}

	free(list);
}

/* Insert a new data element into the end of a linked list */
list_t
*insert_at_foot(list_t *list, data_t value) {
	node_t *new;

	new = (node_t*)malloc(sizeof(*new));
	assert(list!=NULL && new!=NULL);
	strcpy(new->data, value);
	new->next = NULL;

	if (list->foot==NULL) {
		/* This is the first insertion into the list */
		list->head = list->foot = new;
	} else {
		list->foot->next = new;
		list->foot = new;
	}

	return list;
}

