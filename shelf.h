#ifndef SHELF_H
#define SHELF_H

#include "sqlite3.h"
#include "stfl.h"
#include <langinfo.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define RETURN_ERR 0
#define RETURN_OK 1
#define EXIT 2
#define BOOK_ADD_MODE 3
#define BOOK_ADD_MODE_SHELF 4
#define BOOK_EDIT_MODE 5
#define STATE_START 6
#define STATE_NAME 7
#define STATE_AUTHOR 8
#define STATE_SHELF 9

#define SHELVES 0
#define BOOKS 1

#define SHELF_NAME_S 25

#define INIT_CAPACITY 10

#define SQLITE3_DB "my_lib_bin.db"
#define DB_OPEN_MY_LIB "SELECT * FROM Shelves"
#define DB_CREATE_MY_LIB "CREATE TABLE Shelves(Id INT, Shelf BLOB, Title BLOB, Author BLOB);"
#define DB_INSERT "INSERT INTO Shelves(Id, Shelf, Title, Author) VALUES(?, ?, ?, ?);"
#define DB_DELETE_BOOK "DELETE FROM Shelves WHERE Id=?;"
#define DB_DELETE_SHELF "DELETE FROM Shelves WHERE Shelf=?;"
#define DB_MODIFY_SHELF "UPDATE Shelves SET Shelf = ? WHERE Shelf = ?;"

struct b_entry {
    wchar_t* book;
    wchar_t* author;
    wchar_t init_ind_w[10];
    wchar_t real_ind_w[10];
    int db_ind;
    int real_ind;
    int init_ind;
};
struct b_entries {
    wchar_t* shelf;
    wchar_t init_ind_w[10];
    wchar_t real_ind_w[10];
    struct b_entry** books_p;
    struct b_entries* next;
    int size;
    unsigned int capacity;
    int32_t real_ind;
    int32_t init_ind;
};

extern unsigned int w_size;
extern struct b_entries* all_books;
extern int total_b_count;
extern int shelf_stub;
extern int book_stub;
extern int delete_happened;

int init_db();
int add_book_db(struct b_entries* shelf, const wchar_t* b_author, const wchar_t* b_book);
void modify_shelf_db(const wchar_t* shelf_new, const wchar_t* shelf_old);
void delete_book_db(int index);
void close_db();

void init_shelves(struct stfl_form* form, const wchar_t* header, const wchar_t* hints);
struct b_entries* init_data();
int populate_shelves(struct stfl_form* form, struct b_entries* shelf);
void purge_shelves(struct stfl_form* form, struct b_entries* shelf);
int populate_books(struct stfl_form* form, struct b_entries* shelf);
int load_book_mem(struct b_entries* shelf, int offset, unsigned int a_w_str_size,
                    unsigned int b_w_str_size, int db_index, const wchar_t* b_author, const wchar_t* b_book);
void add_book(struct stfl_form* form, struct b_entries* shelf, const wchar_t* b_author, const wchar_t* b_book);
void delete_book(struct stfl_form* form, struct b_entries* shelf, const wchar_t* index, int del_with_shelf);
void add_shelf(struct stfl_form* form, struct b_entries* shelf);
void modify_shelf(struct stfl_form* form, struct b_entries* shelf, const wchar_t* old_shelf_id);
void delete_shelf(struct stfl_form* form, struct b_entries* shelf, const wchar_t* name);
void purge_books(struct stfl_form* form, struct b_entries* shelf);

#endif
