#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> // Pour sleep() sur Linux/Mac
#endif
#include "../include/recommandation.h"

#define DATA_FILE "data.txt"

User users[MAX_USERS];
int user_count = 0;
int current_user_id = -1; // Identifiant de l'utilisateur connecté

int get_user_id(const char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1; // Utilisateur non trouvé
}

Product products[MAX_PRODUCTS] = {
    {"Laptop", 1000.0},
    {"Smartphone", 800.0},
    {"Headphones", 150.0},
    {"Backpack", 50.0},
    {"Smartwatch", 200.0},
    {"Tablet", 400.0}
};

Graph graph;

// Gestion des utilisateurs
void initialize_users(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        while (fscanf(file, "%49s %49s", users[user_count].username, users[user_count].password) == 2) {
            user_count++;
        }
        fclose(file);
    }
}

#define PURCHASES_FILE "purchases.txt"

// Sauvegarder les achats dans un fichier
void save_purchases() {
    FILE *file = fopen(PURCHASES_FILE, "w");
    if (file) {
        for (int i = 0; i < user_count; i++) {
            if (strlen(users[i].username) > 0) { // Vérifier si l'utilisateur existe
                for (int j = 0; j < MAX_PRODUCTS; j++) {
                    if (graph.weights[i][j] > 0) { // Vérifier si l'utilisateur a acheté ce produit
                        fprintf(file, "%s %s %.1f\n", users[i].username, products[j].name, graph.weights[i][j]);
                    }
                }
            }
        }
        fclose(file);
    } else {
        printf("Error: Could not save purchases to file.\n");
    }
}

// Charger les achats depuis un fichier
void load_purchases() {
    FILE *file = fopen(PURCHASES_FILE, "r");
    if (file) {
        char username[50], product_name[50];
        float weight;

        // Initialiser le graphe avant de charger les achats
        initialize_graph();

        while (fscanf(file, "%49s %49s %f", username, product_name, &weight) == 3) {
            int user_id = -1, product_id = -1;

            // Trouver l'ID de l'utilisateur
            for (int i = 0; i < user_count; i++) {
                if (strcmp(users[i].username, username) == 0) {
                    user_id = i;
                    break;
                }
            }

            // Trouver l'ID du produit
            for (int i = 0; i < MAX_PRODUCTS; i++) {
                if (strcmp(products[i].name, product_name) == 0) {
                    product_id = i;
                    break;
                }
            }

            // Ajouter l'achat au graphe
            if (user_id != -1 && product_id != -1) {
                graph.weights[user_id][product_id] = weight; // Remplacer le poids au lieu de l'ajouter
            }
        }
        fclose(file);
    } else {
        printf("No purchases file found. Starting with an empty graph.\n");
    }
}

int add_user(const char *username, const char *password) {
    // Vérifier que les champs ne sont pas vides
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("Error: Username and password cannot be empty.\n");
        return 0; // Échec de l'inscription
    }

    // Vérifier que l'utilisateur n'existe pas déjà
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            printf("Error: Username already exists.\n");
            return 0; // Échec de l'inscription
        }
    }

    // Ajouter l'utilisateur
    strcpy(users[user_count].username, username);
    strcpy(users[user_count].password, password);
    user_count++;
    save_users(DATA_FILE); // Sauvegarder immédiatement après ajout
    return 1; // Inscription réussie
}

int authenticate_user(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return 1; // Connexion réussie
        }
    }
    return 0; // Échec de la connexion
}

void save_users(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file) {
        for (int i = 0; i < user_count; i++) {
            fprintf(file, "%s %s\n", users[i].username, users[i].password);
        }
        fclose(file);
    }
}

// Gestion du magasin
void initialize_store() {
    printf("Store initialized with %d products.\n", MAX_PRODUCTS);
}

// Gestion des graphes
void initialize_graph() {
    memset(graph.weights, 0, sizeof(graph.weights)); // Initialiser toutes les valeurs à 0
    graph.node_count = 0;
    printf("Graph initialized.\n"); // Message de débogage
}

void add_edge(int user_id, int product_id, float weight) {
    if (user_id >= 0 && user_id < MAX_USERS && product_id >= 0 && product_id < MAX_PRODUCTS) {
        graph.weights[user_id][product_id] += weight; // Ajouter le poids de l'achat
        printf("Added purchase: User %d -> Product %d (Weight: %.1f)\n", user_id, product_id, graph.weights[user_id][product_id]);
        save_purchases(); // Sauvegarder immédiatement après modification
    } else {
        printf("Invalid user_id or product_id.\n");
    }
}

void recommend_products(int user_id) {
    printf("Recommended products based on your purchases:\n");
    for (int i = 0; i < MAX_PRODUCTS; i++) {
        if (graph.weights[user_id][i] > 0) {
            printf("- %s (Interest: %.1f)\n", products[i].name, graph.weights[user_id][i]);
        }
    }
    printf("\nPress any key to continue...");
    getchar(); getchar(); // Pause avant de continuer
}

void display_graph() {
    printf("\n=== Relations Graph (User -> Products) ===\n");

    for (int i = 0; i < user_count; i++) {
        if (strlen(users[i].username) > 0) {
            printf("User %s -> ", users[i].username);

            int has_purchases = 0;
            for (int j = 0; j < MAX_PRODUCTS; j++) {
                if (graph.weights[i][j] > 0) {
                    printf("%s (Quantity: %.1f) ", products[j].name, graph.weights[i][j]);
                    has_purchases = 1;
                }
            }

            if (!has_purchases) {
                printf("No purchases yet.");
            }
            printf("\n");
        }
    }

    printf("\nPress any key to continue...");
    getchar();
}

void clear_all_data() {
    // Vider les utilisateurs
    memset(users, 0, sizeof(users)); // Réinitialiser le tableau des utilisateurs
    user_count = 0; // Réinitialiser le compteur d'utilisateurs
    current_user_id = -1; // Réinitialiser l'ID de l'utilisateur connecté

    // Vider les achats (graphe)
    memset(graph.weights, 0, sizeof(graph.weights)); // Réinitialiser le graphe
    graph.node_count = 0;

    // Vider les fichiers de données
    FILE *file = fopen(DATA_FILE, "w");
    if (file) {
        fclose(file); // Crée un fichier vide
    } else {
        printf("Error: Could not clear user data file.\n");
    }

    file = fopen(PURCHASES_FILE, "w");
    if (file) {
        fclose(file); // Crée un fichier vide
    } else {
        printf("Error: Could not clear purchases data file.\n");
    }

    printf("All data has been cleared successfully!\n");
    #ifdef _WIN32
     Sleep(2000); // Pause de 2 secondes sur Windows
    #else
     sleep(2); // Pause de 2 secondes sur Linux/Mac
    #endif
    }

// Efface l'écran
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Programme principal
void add_predefined_users() {
    add_user("user1", "password1");
    add_user("user2", "password2");
}

// Fonction pour afficher le panier
void view_cart(int user_id) {
    printf("\n=== Your Cart ===\n");
    int has_purchases = 0;
    for (int i = 0; i < MAX_PRODUCTS; i++) {
        if (graph.weights[user_id][i] > 0) {
            printf("- %s (Quantity: %.1f)\n", products[i].name, graph.weights[user_id][i]);
            has_purchases = 1;
        }
    }
    if (!has_purchases) {
        printf("Your cart is empty.\n");
    }
    printf("\nPress any key to continue...");
    getchar(); getchar(); // Pause avant de continuer
}

int get_personal_recommendation() {
    if (current_user_id == -1) {
        printf("No user is currently logged in.\n");
        return -1;
    }

    float max_score = 0.0;
    int recommended_product = -1;

    for (int product_id = 0; product_id < MAX_PRODUCTS; product_id++) {
        float score = graph.weights[current_user_id][product_id];

        // Trouver le produit avec le score le plus élevé
        if (score > max_score) {
            max_score = score;
            recommended_product = product_id;
        }
    }

    return recommended_product;
}

int get_most_popular_product() {
    int max_count = 0;
    int most_popular_product = -1;

    for (int product_id = 0; product_id < MAX_PRODUCTS; product_id++) {
        int count = 0;

        // Parcourir toutes les lignes pour compter les utilisateurs ayant ce produit
        for (int user_id = 0; user_id < user_count; user_id++) {
            if (graph.weights[user_id][product_id] > 0) {
                count++;
            }
        }

        // Mise à jour si le produit est plus populaire
        if (count > max_count) {
            max_count = count;
            most_popular_product = product_id;
        }
    }

    return most_popular_product;
}

void login(const char *username) {
    current_user_id = get_user_id(username); // Récupérer l'identifiant de l'utilisateur
    printf("User  '%s' logged in.\n", username);
}

void store_menu(const char *username) {
    login(username); // Identifier l'utilisateur connecté
    int choice;
    int valid_input;

    printf("\nWelcome to the store, %s!\n", username);

    // Recommandation générale
    int general_product = get_most_popular_product();
    if (general_product != -1) {
        printf("\nPopular product (for all users):\n");
        printf("- %s ($%.2f)\n", products[general_product].name, products[general_product].price);
    }

    // Recommandation personnalisée
    int personal_product = get_personal_recommendation();
    if (personal_product != -1) {
        printf("\nRecommended for you:\n");
        printf("- %s ($%.2f)\n", products[personal_product].name, products[personal_product].price);
    }

    // Afficher les produits disponibles
    printf("\n=== Available Products ===\n");
    for (int i = 0; i < MAX_PRODUCTS; i++) {
        printf("%d. %s ($%.2f)\n", i + 1, products[i].name, products[i].price);
    }

    printf("\nChoose products to buy (enter product numbers separated by spaces, or 0 to exit): ");
    while (1) {
        valid_input = scanf("%d", &choice);
        if (valid_input != 1) {
            // Gestion des caractères non numériques
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Vider le buffer d'entrée
            continue;
        }

        if (choice == 0) break;

        if (choice > 0 && choice <= MAX_PRODUCTS) {
            printf("You bought %s for $%.2f!\n", products[choice - 1].name, products[choice - 1].price);
            if (current_user_id != -1) {
                add_edge(current_user_id, choice - 1, 1.0); // Ajouter au panier
            } else {
                printf("Error: User is not logged in.\n");
                break;
            }
        } else {
            printf("Invalid product number. Please choose a number between 1 and %d.\n", MAX_PRODUCTS);
        }
    }

    printf("\nPress any key to return to the menu...");
    getchar(); getchar(); // Pause avant de continuer
}

int main() {
    int choice;
    char username[50], password[50];
    int is_logged_in = 0; // 0 = déconnecté, 1 = connecté
    int current_user_id = -1; // ID de l'utilisateur connecté

    initialize_users(DATA_FILE);
    initialize_store();
    initialize_graph();
    load_purchases(); // Charger les achats depuis le fichier
    add_predefined_users();

    while (1) {
        clear_screen();
        printf("\n");
        printf("╔════════════════════════════════════════════════════════════╗\n");
        printf("║                      🛒 MINI STORE 🛒                      ║\n");
        printf("╠════════════════════════════════════════════════════════════╣\n");

        if (!is_logged_in) {
            printf("║ 1. Register          │ Create a new account                ║\n");
            printf("║ 2. Login             │ Access your account                 ║\n");
            printf("║ 3. Exit              │ Quit the program                    ║\n");
        } else {
            printf("║ 1. View Cart         │ See your purchased items            ║\n");
            printf("║ 2. Shop              │ Browse and buy products             ║\n");
            printf("║ 3. Display Graph     │ View purchase relationships         ║\n");
            printf("║ 4. Logout            │ Log out from your account           ║\n");
            printf("║ 5. Clear All Data    │ Reset all data (users & products)   ║\n");
            printf("║ 6. Exit              │ Quit the program                    ║\n");
        }

        printf("╚════════════════════════════════════════════════════════════╝\n");
        printf("\nChoose an option: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
        case 1:
            if (!is_logged_in) {
                // Enregistrement
                clear_screen();
                char username[50];
                int registration_success = 0;

                while (!registration_success) {
                    printf("\nEnter username: ");
                    fgets(username, sizeof(username), stdin);
                    username[strcspn(username, "\n")] = '\0'; // Supprimer le saut de ligne

                    printf("Enter password: ");
                    fgets(password, sizeof(password), stdin);
                    password[strcspn(password, "\n")] = '\0'; // Supprimer le saut de ligne

                    if (add_user(username, password)) {
                        printf("User registered successfully!\n");
                        registration_success = 1; // Sortir de la boucle
                    } else {
                        printf("Please try again.\n");
                    }
                }
                #ifdef _WIN32
                Sleep(2000); // Pause de 2 secondes sur Windows
                #else
                sleep(2); // Pause de 2 secondes sur Linux/Mac
                #endif
            } else {
                // Afficher le panier
                clear_screen();
                view_cart(current_user_id);
            }
            break;

        case 2:
            if (!is_logged_in) {
                // Connexion
                clear_screen();
                int login_success = 0;

                while (!login_success) {
                    printf("\nEnter username: ");
                    fgets(username, sizeof(username), stdin);
                    username[strcspn(username, "\n")] = '\0'; // Supprimer le saut de ligne

                    printf("Enter password: ");
                    fgets(password, sizeof(password), stdin);
                    password[strcspn(password, "\n")] = '\0'; // Supprimer le saut de ligne

                    if (authenticate_user(username, password)) {
                        printf("Login successful!\n");
                        is_logged_in = 1;
                        current_user_id = get_user_id(username); // Mettre à jour l'ID de l'utilisateur
                        login_success = 1; // Sortir de la boucle
                    } else {
                        printf("Invalid username or password. Please try again.\n");
                    }
                }
                #ifdef _WIN32
                Sleep(2000); // Pause de 2 secondes sur Windows
                #else
                sleep(2); // Pause de 2 secondes sur Linux/Mac
                #endif
            } else {
                // Faire des courses
                clear_screen();
                store_menu(users[current_user_id].username);
            }
            break;

        case 3:
            if (!is_logged_in) {
                // Quitter
                printf("Exiting the program. Goodbye!\n");
                exit(0);
            } else {
                // Afficher le graphe
                clear_screen();
                display_graph();
            }
            break;

        case 4:
            if (is_logged_in) {
                // Déconnexion
                is_logged_in = 0;
                current_user_id = -1;
                printf("You have been logged out.\n");
                #ifdef _WIN32
                Sleep(2000); // Pause de 2 secondes sur Windows
                #else
                sleep(2); // Pause de 2 secondes sur Linux/Mac
                #endif
            } else {
                printf("Invalid option. Please try again.\n");
            }
            break;

        case 5:
            if (is_logged_in) {
                clear_all_data();
            } else {
                printf("Invalid option. Please try again.\n");
            }
            break;

        case 6:
            printf("Exiting the program. Goodbye!\n");
            exit(0);

        default:
            printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
