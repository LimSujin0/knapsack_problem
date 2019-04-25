
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

//my program works for Greedy and Dynamic Programming solution and Branch Bound
struct item {
	int value = 0;
	int weight = 0;
	float value_per_weight = 0;
};

typedef struct b_node { //binary node
	bool promising = true;
	int item_num = 0;
	int benefit = 0;
	int weight = 0;
	int bound = 0;
	struct b_node* left_node = NULL;
	struct b_node* right_node = NULL;
}b_node;

typedef struct s_node {
	struct b_node* key = NULL;
	struct s_node* next = NULL;
}s_node;
s_node *head, *tail;

void init_stack() {
	head = (s_node*)malloc(sizeof(s_node));
	tail = (s_node*)malloc(sizeof(s_node));
	head->next = tail;
	tail->next = head;
}

b_node* push(b_node* k) {
	s_node* t = (s_node*)malloc(sizeof(s_node));
	t->key = k;
	t->next = head->next;
	head->next = t;
	return k;
}

b_node* pop() {
	s_node* t = (s_node*)malloc(sizeof(s_node));
	t = head->next;
	b_node* k = t->key;
	head->next = t->next;
	free(t);
	return k;
}

void swap(item* items, int l, int r);
void quick(item* items, int left, int right);
struct timeval getMicrosecond();

int dynamic(item*items, int capacity, int item_num);
int greedy(item*items, int capacity, int item_num);

int get_bound(item*items, b_node* node, int capacity, int item_num);
void new_child(item*items, b_node* parent, b_node* l_child, b_node* r_child, int* max_value, int capacity, int item_num);
void check_promising(b_node* node, int capacity, int max_value);
void sort_stack();
int bb(item* items, int capacity, int item_num);

int main() {
	FILE *fp;
	fp = fopen("output.txt", "w");
	int item_nums[9] = {10, 100, 500, 1000, 3000, 5000, 7000, 9000, 10000};
	int max_val_greedy = 0, max_val_dp = 0, max_val_bb = 0;

	//long s1, e1, t1, s2, e2, t2;
	struct timeval s0, s1, s2, s3;
	struct timeval e0, e1, e2, e3;
	struct timeval t0, t1, t2, t3;


	//get random number of value and weight
	for (int j = 0; j < 9; j++) {
		double term1 = 0, term2 = 0, term3 = 0;
		int item_num = item_nums[j];
		int capacity = item_num * 40;
		item* items = (item*)malloc(sizeof(item)*item_num);
		srand((unsigned)time(NULL));
		for (int i = 0; i < item_num; i++) {
			items[i].value = (rand() % 300) + 1;
			items[i].weight = (rand() % 100) + 1;
			items[i].value_per_weight = (float)items[i].value / (float)items[i].weight;
		}
		//sorting by value/weight
		s0 = getMicrosecond();
		quick(items, 0, item_num - 1);
		e0 = getMicrosecond();
		t0.tv_usec = e0.tv_usec - s0.tv_usec;
		//printf("time0 : %d\n", t0.tv_usec);


		//greedy
		s1 = getMicrosecond();
		max_val_greedy = greedy(items, capacity, item_num);
		e1 = getMicrosecond();
		t1.tv_usec = e1.tv_usec - s1.tv_usec;
		//printf("time1 : %d\n", t1.tv_usec);
		term1 = (double)t1.tv_usec / 1000;

		//dynamic programming
		s2 = getMicrosecond();
		max_val_dp = dynamic(items, capacity, item_num);
		e2 = getMicrosecond();
		t2.tv_usec = e2.tv_usec - s2.tv_usec;
		//printf("time2 : %d\n", t2.tv_usec);
		term2 = (double)t2.tv_usec /1000;


		//branch and bound
		s3 = getMicrosecond();
		max_val_bb = bb(items, capacity, item_num);
		e3 = getMicrosecond();
		t3.tv_usec = e3.tv_usec - s3.tv_usec;
		//printf("time3 : %d\n", t3.tv_usec);
		term3 = (double)t3.tv_usec /1000;

		printf("\n***item_num : %d***\ngreedy : %d(%.3f (ms))\ndynamic programming : %d(%.3f (ms))\nbranch and bound : %d(%.3f(ms))\n",
			item_num, max_val_greedy, term1, max_val_dp, term2, max_val_bb, term3);

		fprintf(fp, "item_num : %d\nalgorithm\tbenefit\ttime(ms)\ngreedy algorithm\t%d\t%.3f\t\ndynamic algorithm\t%d\t%.3f\nbranch bound\t%d\t%.3f\n\n",
			item_num, max_val_greedy, term1, max_val_dp, term2, max_val_bb, term3);
	}
	fclose(fp);
	system("pause");
}

struct timeval getMicrosecond() {
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;
	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
	//printf(" %d:%02d:%02d %d \n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv.tv_usec);
	struct timeval total;
	total.tv_usec = tm->tm_min * 60 * 1000000 + tm->tm_sec * 1000000 + tv.tv_usec;
	return total;
}


int bb(item*items, int capacity, int item_num) {
	int max_val = 0;

	//initialize root node
	b_node* root = (b_node*)malloc(sizeof(b_node));
	root->promising = true;
	root->item_num = -1;
	root->benefit = 0;
	root->weight = 0;
	root->left_node = NULL;
	root->right_node = NULL;
	//initialize the stack
	init_stack();

	b_node* parent = root;
	//push(parent);
	get_bound(items, parent, capacity, item_num);
	max_val = parent->benefit;
	//get max_benefit
	while (head->next != tail || parent==root) {
		check_promising(parent, capacity, max_val);
		if (parent->promising) {
			b_node* l_child = (b_node*)malloc(sizeof(b_node));
			b_node* r_child = (b_node*)malloc(sizeof(b_node));
			new_child(items, parent, l_child, r_child, &max_val, capacity, item_num);

			if (parent->left_node->promising) {
				push(parent->left_node);
				sort_stack();
			}
			if (parent->right_node->promising) {
				push(parent->right_node);
				sort_stack();
			}
			parent->promising = false; //to expand node, temporary change parent node as non-promising
		}
		else {
			free(parent);
			parent = pop();
			if (max_val < parent->benefit) max_val = parent->benefit;
		}
	}
	return max_val;
}

void sort_stack() {
	s_node* target = head->next;
	s_node* comp = head->next->next;
	s_node* prev = head;
	while (comp != tail && target->key->bound < comp->key->bound) {
		prev = comp;
		comp = comp->next;
	}
	head->next = head->next->next;
	target->next = comp;
	prev->next = target;
}

void new_child(item*items, b_node* parent, b_node* l_child, b_node* r_child, int* max_val, int capacity, int item_num) {
	int c_item_num = (parent->item_num) + 1; //child item number
	if (c_item_num > item_num - 1) {
		l_child->promising = false;
		r_child->promising = false;
		parent->left_node = l_child;
		parent->right_node = r_child;
	}
	else {
		l_child->item_num = c_item_num; //get c_th item
		l_child->benefit = parent->benefit + items[c_item_num].value;
		l_child->weight = parent->weight + items[c_item_num].weight;
		l_child->left_node = NULL;
		l_child->right_node = NULL;

		r_child->item_num = c_item_num; //do not get c_th item
		r_child->weight = parent->weight;
		r_child->benefit = parent->benefit;
		r_child->left_node = NULL;
		r_child->right_node = NULL;

		get_bound(items, l_child, capacity, item_num);
		get_bound(items, r_child, capacity, item_num);
		check_promising(l_child, capacity, *max_val);
		check_promising(r_child, capacity, *max_val);

		parent->left_node = l_child;
		parent->right_node = r_child;
	}
}

void check_promising(b_node* node, int capacity, int max_val) {
	if (node->weight > capacity) {
		node->promising = false;
	}
	else if (node->bound <= max_val) {
		node->promising = false;
	}
	else if (node->right_node != 0 && node->left_node != 0) {
		if (!(node->right_node->promising && node->left_node->promising)) {
			node->promising = false;
		}
	}
	else {
		node->promising = true;
	}
}

int get_bound(item*items, b_node* node, int capacity, int item_num) {
	int remain = capacity - (node->weight);
	int bound = node->benefit;
	int i = 0;
	i = (node->item_num) + 1;
	if (i > item_num - 1) {
		node->bound = bound;
		return 0;
	}
	while (remain - items[i].weight >= 0 && i < item_num - 1) {
		bound += items[i].value;
		remain -= items[i].weight;
		i++;
	}
	if (i > item_num - 1) node->bound = bound;
	else {
		bound += remain * items[i].value_per_weight;
		node->bound = bound;
	}
	return 0;
}


int greedy(item*items, int capacity, int item_num) {
	int max_val = 0; //max_value that we can get
	int remain_weight = capacity; //remain weight
	int i = 0;
	while (remain_weight - items[i].weight && i < item_num - 1) { //조건 합치자
		max_val += items[i].value;
		remain_weight -= items[i].weight;
		i++;
	}
	if (i == item_num - 1 && remain_weight >= items[i].weight) {
		max_val += items[i].value;
	}
	else {
		max_val += remain_weight * items[i].value_per_weight;
	}
	return max_val;
}

int dynamic(item* items, int capacity, int item_num) {
	int max_value = 0; //max_value
	int i_total_weight = 0; //i_th total weight
	//initialize result table
	int **result = (int**)malloc(sizeof(int*)*(item_num+1));
	int max_weight=0;
	result[0] = (int*)malloc(sizeof(int)*(capacity+1));
	for (int i = 0; i < capacity+1; i++) result[0][i] = 0;

	//get value in the table
	for (int i = 1; i < item_num+1; i++) {
		result[i] = (int*)malloc(sizeof(int)*(capacity+1));
		result[i][0] = 0;
		i_total_weight += items[i-1].weight;
		for (int w = 1; w < capacity+1; w++) { //더 효율적인 index 존재
			result[i][w] = 0;
			if (items[i - 1].weight <= w) {
				if ((items[i - 1].value + result[i-1][(w - items[i-1].weight)]) > result[i - 1][w])
					result[i][w] = items[i - 1].value + result[i - 1][(w - items[i - 1].weight)];
				else
					result[i][w] = result[i - 1][w];
			}
			else {
				result[i][w] = result[i - 1][w];
			}
			max_weight = w;
		}
		free(result[i - 1]);
	}
	max_value = result[item_num ][max_weight];
	free(result[item_num]);
	free(result);
	return max_value;
}

void quick(item* items, int left, int right) {
	int l = left;
	int r = right;
	float pivot = items[(left + right) / 2].value_per_weight;

	while (l <= r) {
		while (items[l].value_per_weight > pivot)
			l++;
		while (items[r].value_per_weight < pivot)
			r--;
		if (l <= r) {
			if (l != r)
				swap(items, l, r);
			if (l == r && r == left)
				swap(items, r, (left + right) / 2);
			if (l == r && l == right) {
				swap(items, l, (left + right) / 2);
			}
			l++;
			r--;
		}
	}
	if (left < r)
		quick(items, left, r);
	if (l < right)
		quick(items, l, right);
}


void swap(item* items, int l, int r) {
	item tmp = items[l];
	items[l] = items[r]; //need to exchange pointers
	items[r] = tmp;
}
