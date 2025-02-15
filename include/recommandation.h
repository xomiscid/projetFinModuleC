#ifndef RECOMMANDATION_H
#define RECOMMANDATION_H

#define MAX_USERS 100
#define MAX_PRODUCTS 6
#define MAX_NODES 100

typedef struct {
    char username[50];
    char password[50];
} User;

typedef struct {
    char name[50];
    double price;
} Product;

typedef struct {
    float weights[MAX_NODES][MAX_NODES];
    int node_count;
} Graph;

void initialize_users(const char *filename);
int add_user(const char *username, const char *password);
int authenticate_user(const char *username, const char *password);
void save_users(const char *filename);

void initialize_store();
void store_menu(const char *username);

void initialize_graph();
void add_edge(int user_id, int product_id, float weight);
void recommend_products(int user_id);
void display_graph();
void free_graph();

void clear_screen();

#endif
