#include "shelf.h"

static int db_process_read(int db_index, char** blobs, unsigned int* sizes);
static int db_persist_book(sqlite3_stmt **stmt, const wchar_t* shelf, const wchar_t* book, const wchar_t* author, int index);

static const wchar_t* s_read = L"read";
static const wchar_t* s_reading = L"reading";
static const wchar_t* s_to_read = L"to read";
static const wchar_t* temp_text_0_b = L"Street Photograpy Now";
static const wchar_t* temp_text_0_a = L"Howarth, Sophie";
static const wchar_t* temp_text_1_b = L"1984";
static const wchar_t* temp_text_1_a = L"Orwell, George";
static const wchar_t* temp_text_2_b = L"The Idiot";
static const wchar_t* temp_text_2_a = L"Dostoyevsky, Fyodor";

static sqlite3* db;
static char* db_err_msg = 0;

int total_b_count;

int init_db()
{
    sqlite3_stmt *p_stmt;
    sqlite3_stmt *p_stmt_init;
    unsigned char* pz_blob[3];
    int* pn_blob = malloc(sizeof(int) * 3);
    total_b_count =0;
    int read_from_db = 0;
    int res = sqlite3_open_v2(SQLITE3_DB, &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    if (res != SQLITE_OK) {
        fprintf(stderr, "Cannot open sqlite3 database file: %s\n",
            sqlite3_errmsg(db));
        goto DB_RET_ON_ERR;
    }
    res = sqlite3_prepare(db, DB_OPEN_MY_LIB, -1, &p_stmt, 0);
    if (res == SQLITE_ERROR) {
        res = sqlite3_exec(db, DB_CREATE_MY_LIB, 0, 0, &db_err_msg);
        if (res != SQLITE_OK) {
            goto DB_RET_ON_ERR;
        } else {
            res = db_persist_book(&p_stmt_init, s_read, temp_text_0_b, temp_text_0_a, 0);
            if (res != SQLITE_OK) {
                goto DB_RET_ON_ERR;
            }
            res = db_persist_book(&p_stmt_init, s_to_read, temp_text_1_b, temp_text_1_a, 1);
            if (res != SQLITE_OK) {
                goto DB_RET_ON_ERR;
            }
            res = db_persist_book(&p_stmt_init, s_reading, temp_text_2_b, temp_text_2_a, 2);
            if (res != SQLITE_OK) {
                goto DB_RET_ON_ERR;
            }
            res = sqlite3_prepare(db, DB_OPEN_MY_LIB, -1, &p_stmt, 0);
            if (res != SQLITE_OK) {
                goto DB_RET_ON_ERR;
            }
        }
    }
    for(;;) {
        res = sqlite3_step(p_stmt);
        if (res == SQLITE_ROW) {
            read_from_db = 1;
            for(int i = 1; i < 4; i++) {
                *(pn_blob + i - 1) = sqlite3_column_bytes(p_stmt, i);
                pz_blob[i - 1] = (unsigned char *) malloc(*(pn_blob + i - 1));
                memcpy(pz_blob[i - 1], sqlite3_column_blob(p_stmt, i), *(pn_blob + i - 1));
            }
            int index = sqlite3_column_int(p_stmt, 0);
            db_process_read(index, (char**)pz_blob, (unsigned int*)pn_blob);
            total_b_count++;
        } else {
            break;
        }
    }
    sqlite3_finalize(p_stmt);
    if (read_from_db) {
        free(pz_blob[0]);
        free(pz_blob[1]);
        free(pz_blob[2]);
    }
    free(pn_blob);
    return RETURN_OK;
DB_RET_ON_ERR:
    if (db_err_msg != 0) {
        fprintf(stderr, "SQL error: %s\n", db_err_msg);
        sqlite3_free(db_err_msg);
    }
    sqlite3_close(db);
    return RETURN_ERR;
}

void delete_book_db(int index)
{
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare(db, DB_DELETE_BOOK, -1, &stmt, 0);
    if(res != SQLITE_OK) {
        return;
    }
    sqlite3_bind_int(stmt, 1, index);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void modify_shelf_db(const wchar_t* shelf_new, const wchar_t* shelf_old)
{
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare(db, DB_MODIFY_SHELF, -1, &stmt, 0);
    if(res != SQLITE_OK) {
        return;
    }
    sqlite3_bind_blob(stmt, 1, shelf_new, (wcslen(shelf_new) + 1) * w_size, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, shelf_old, (wcslen(shelf_old) + 1) * w_size, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void close_db() 
{
    sqlite3_close(db);
}

static int db_process_read(int db_index, char** blobs, unsigned int* sizes)
{
    int ret_val = RETURN_OK;
    unsigned int s_str_size = *sizes;
    unsigned int s_w_str_size = s_str_size * w_size;
    unsigned int a_str_size = *(sizes + 2);
    unsigned int a_w_str_size = a_str_size * w_size;
    unsigned int b_str_size = *(sizes + 1);
    unsigned int b_w_str_size = b_str_size * w_size;
    wchar_t* next_shelf = 0;
    wchar_t* next_author = 0;
    wchar_t* next_book = 0;
    struct b_entries* load_books = all_books;
    next_shelf = malloc(s_w_str_size);
    next_book = malloc(b_w_str_size);
    next_author = malloc(a_w_str_size);
    if (next_shelf != 0 && next_author != 0 && next_book != 0) {
        for(int i = 0; i < s_str_size / w_size; i++) {
            mbtowc(next_shelf + i, (blobs[0] + i * w_size), w_size);
        }
        for(int i = 0; i < b_str_size / w_size; i++) {
            mbtowc(next_book + i, (blobs[1] + i * w_size), w_size);
        }
        for(int i = 0; i < a_str_size / w_size; i++) {
            mbtowc(next_author + i, (blobs[2] + i * w_size), w_size);
        }
        for (unsigned int i = 0;;i++) {
            if (load_books->shelf == 0) {
                load_books->shelf = malloc(s_w_str_size);
                if (load_books->shelf != 0) {
                    wmemcpy(load_books->shelf, next_shelf, wcslen(next_shelf) + 1);
                    load_books->init_ind = i;
                    load_books->real_ind = i;
                    swprintf(load_books->init_ind_w, 10, L"%d",load_books->real_ind);
                    swprintf(load_books->real_ind_w, 10, L"%d",load_books->real_ind);
                    break;
                } else {
                    ret_val = RETURN_ERR;
                    goto EXIT_READ;
                }
            } else if (!wcsncmp(load_books->shelf, next_shelf, SHELF_NAME_S)) {
                break;
            } else if (load_books->next != 0) {
                load_books = load_books->next;
            } else {
                load_books->next = init_data();
                if (load_books->next != 0) {
                    load_books = load_books->next;
                } else {
                    ret_val = RETURN_ERR;
                    goto EXIT_READ;
                }
            }
        }
        if(!load_book_mem(load_books, load_books->size, a_w_str_size, b_w_str_size,
                        db_index, next_author, next_book)) {
            ret_val = RETURN_ERR;
            goto EXIT_READ;
        }
    } else {
        ret_val = RETURN_ERR;
    }
EXIT_READ:
    free(next_shelf);
    free(next_author);
    free(next_book);
    return ret_val;
}

int add_book_db(struct b_entries* shelf, const wchar_t* b_author, const wchar_t* b_book)
{
    sqlite3_stmt *stmt;
    ++total_b_count;
    if (db_persist_book(&stmt, shelf->shelf, b_book, b_author, total_b_count) != SQLITE_OK) {
        return RETURN_ERR;
    }
    return RETURN_OK;
}

static int db_persist_book(sqlite3_stmt **stmt, const wchar_t* shelf, const wchar_t* book, const wchar_t* author, int index)
{
    int res = sqlite3_prepare(db, DB_INSERT, -1, stmt, 0);
    if(res != SQLITE_OK) {
        goto ERROR;
    }
    sqlite3_bind_int(*stmt, 1, index);
    sqlite3_bind_blob(*stmt, 2, shelf, (wcslen(shelf) + 1) * w_size, SQLITE_STATIC);
    sqlite3_bind_blob(*stmt, 3, book, (wcslen(book) + 1) * w_size, SQLITE_STATIC);
    sqlite3_bind_blob(*stmt, 4, author, (wcslen(author) + 1) * w_size, SQLITE_STATIC);
    sqlite3_step(*stmt);
    sqlite3_finalize(*stmt);
    ERROR:
    return res;
}

