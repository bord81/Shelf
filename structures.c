#include "shelf.h"

static const wchar_t* prog_name = L"Yet another book shelf";
static const wchar_t* breaker = L" / ";
static int digits_increase(int size);
int shelf_stub = 0;
int book_stub = 0;

void init_shelves(struct stfl_form* form, const wchar_t* header, const wchar_t* hints)
{
    stfl_set(form, L"prog_name", prog_name);
    stfl_set(form, L"browsestate", header);
    stfl_set(form, L"help", hints);
}

struct b_entries* init_data()
{
    struct b_entries* shelf = malloc(sizeof(struct b_entries));
    if (shelf != 0) {
        shelf->size = 0;
        shelf->shelf = NULL;
        shelf->next = 0;
        shelf->capacity = INIT_CAPACITY;
        shelf->books_p = malloc(shelf->capacity * sizeof(struct b_entry*));
        if (shelf->books_p == 0) {
            return 0;
        }
    }
    return shelf;
}

int load_book_mem(struct b_entries* shelf, int offset, unsigned int a_w_str_size,
                    unsigned int b_w_str_size, int db_index, const wchar_t* b_author, const wchar_t* b_book)
{
    if ((shelf->size + 1) > (shelf->capacity * 3 / 4)) {
        struct b_entry** n_entry;
        if ((n_entry = realloc(shelf->books_p, (shelf->capacity * 2) * sizeof(struct b_entry*)))) {
            shelf->books_p = n_entry;
            shelf->capacity *= 2;
        } else {
            return RETURN_ERR;
        }
    }
    *(shelf->books_p + offset)  = malloc(sizeof(struct b_entry));
    (*(shelf->books_p + offset))->author = malloc(a_w_str_size);
    (*(shelf->books_p + offset))->book = malloc(b_w_str_size);
    wmemcpy((*(shelf->books_p + offset))->author, b_author, wcslen(b_author) + 1);
    wmemcpy((*(shelf->books_p + offset))->book, b_book, wcslen(b_book) + 1);
    if (delete_happened) {
        int deleted_count = 0;
        for (int i = 0; i < shelf->size; i++) {
            if ((*(shelf->books_p + i))->real_ind == -1) {
                deleted_count++;
            }
        }
        (*(shelf->books_p + offset))->init_ind = shelf->size - deleted_count;
        (*(shelf->books_p + offset))->real_ind = shelf->size - deleted_count;
    } else {
        (*(shelf->books_p + offset))->init_ind = shelf->size;
        (*(shelf->books_p + offset))->real_ind = shelf->size;
    }
    swprintf((*(shelf->books_p + offset))->init_ind_w, 10, L"%d", (*(shelf->books_p + offset))->init_ind);
    swprintf((*(shelf->books_p + offset))->real_ind_w, 10, L"%d", (*(shelf->books_p + offset))->real_ind);
    (*(shelf->books_p + offset))->db_ind = db_index;
    shelf->size++;
    return RETURN_OK;
}

int populate_shelves(struct stfl_form* form, struct b_entries* shelf)
{
    int ret_val = RETURN_OK;
    if(shelf->size == 0) {
        if(shelf_stub == 0) {
            int stub_line_size = 30 * w_size;
            wchar_t* buf = malloc(stub_line_size);
            swprintf(buf, stub_line_size, L"{listitem[%d] text:\"%ls\"}", -1, L" ");
            stfl_modify(form, L"shelves", L"append", buf);
            free(buf);
            shelf_stub++;
        }
        goto EXIT_P_SHELVES;
    }
    struct b_entries* temp_shelf = shelf;
    while(temp_shelf != 0) {
        if(temp_shelf->real_ind > -1) {
            size_t total_size = (wcslen(temp_shelf->shelf) + digits_increase(temp_shelf->real_ind)) * w_size;
            wchar_t* buf = malloc(total_size);
            if(buf == 0){
                ret_val = RETURN_ERR;
                goto EXIT_P_SHELVES;
            }
            swprintf(buf, total_size, L"{listitem[%d] text:\"%ls\"}", temp_shelf->real_ind,temp_shelf->shelf);
            stfl_modify(form, L"shelves", L"append", buf);
            free(buf);
    }
        temp_shelf = temp_shelf->next;
    }
    EXIT_P_SHELVES:
    return ret_val;
}

void purge_shelves(struct stfl_form* form, struct b_entries* shelf) {
    while(shelf != 0) {
        if (shelf->real_ind > -1) {
            if (shelf->real_ind != shelf->init_ind) {
                stfl_modify(form, shelf->init_ind_w, L"delete", 0);
                swprintf(shelf->init_ind_w, 10, L"%d", shelf->real_ind);
            } else {
                stfl_modify(form, shelf->real_ind_w, L"delete", 0);
            }
        }
        shelf = shelf->next;
    }
}

int populate_books(struct stfl_form* form, struct b_entries* shelf)
{
    int digits_start = 1;
    int digits_curr = 0;
    int digits_increase = 0;
    if(shelf->size == 0) {
        if(book_stub == 0) {
            int stub_line_size = 30 * w_size;
            wchar_t* buf = malloc(stub_line_size);
            swprintf(buf, stub_line_size, L"{listitem[%d] text:\"%ls%ls%ls\"}", -1, L" ", L" ", L" ");
            stfl_modify(form, L"books", L"append", buf);
            free(buf);
            book_stub++;
        }
    } else {
        for(size_t i = 0; i < shelf->size; i++) {
            digits_curr++;
            if((digits_curr / digits_start) == 10) {
                digits_start *=10;
                digits_increase++;
            }
            if((*(shelf->books_p + i))->real_ind > -1) {
            size_t total_size = (wcslen((*(shelf->books_p + i))->book) + wcslen((*(shelf->books_p + i))->author)
                    + SHELF_NAME_S + digits_increase) * w_size;
            wchar_t* buf = malloc(total_size);
            if(buf == 0) {
                return RETURN_ERR;
            }
            swprintf(buf, total_size, L"{listitem[%d] text:\"%ls%ls%ls\"}", (*(shelf->books_p + i))->real_ind,
                    (*(shelf->books_p + i))->book, breaker, (*(shelf->books_p + i))->author);
            stfl_modify(form, L"books", L"append", buf);
            free(buf);
            }
        }
    }
    return RETURN_OK;
}

void add_book(struct stfl_form* form, struct b_entries* shelf, const wchar_t* b_author, const wchar_t* b_book)
{
    size_t total_size = (wcslen(b_book) + wcslen(b_author) + digits_increase(shelf->size)) * w_size;
    wchar_t* buf = malloc(total_size);
    if (buf == 0) {
        return;
    }
    swprintf(buf, total_size, L"{listitem[%d] text:\"%ls%ls%ls\"}", (*(shelf->books_p + shelf->size - 1))->real_ind,
                    (*(shelf->books_p + shelf->size - 1))->book, breaker, (*(shelf->books_p + shelf->size - 1))->author);
    stfl_modify(form, L"books", L"append", buf);
    free(buf);
}

void add_shelf(struct stfl_form* form, struct b_entries* shelf)
{
    size_t total_size = (wcslen(shelf->shelf) + digits_increase(shelf->real_ind)) * w_size;
    wchar_t* buf = malloc(total_size);
    if (buf == 0) {
        return;
    }
    swprintf(buf, total_size, L"{listitem[%d] text:\"%ls\"}", shelf->real_ind, shelf->shelf);
    stfl_modify(form, L"shelves", L"append", buf);
    free(buf);
}

void modify_shelf(struct stfl_form* form, struct b_entries* shelf, const wchar_t* old_shelf_id)
{
    size_t total_size = (wcslen(shelf->shelf) + digits_increase(shelf->real_ind)) * w_size;
    wchar_t* buf = malloc(total_size);
    if (buf == 0) {
        return;
    }
    swprintf(buf, total_size, L"{listitem[%d] text:\"%ls\"}", shelf->real_ind, shelf->shelf);
    stfl_modify(form, old_shelf_id, L"replace", buf);
    free(buf);
}

void delete_shelf(struct stfl_form* form, struct b_entries* shelf, const wchar_t* name)
{
     int found_del = 0;
     while(shelf != 0) {
        if (!wcscmp(shelf->shelf, name) && !found_del) {
            found_del = 1;
            for(int i = 0; i < shelf->size; i++) {
                delete_book(form, shelf, (*(shelf->books_p + i))->real_ind_w, 1);
            }
            stfl_modify(form, shelf->init_ind_w, L"delete", 0);
            shelf->real_ind = -1;
            swprintf(shelf->real_ind_w, 10, L"%d", shelf->real_ind);

        } else if (found_del) {
            shelf->real_ind--;
            swprintf(shelf->real_ind_w, 10, L"%d", shelf->real_ind);
        }
        shelf = shelf->next;
    }
}

void delete_book(struct stfl_form* form, struct b_entries* shelf, const wchar_t* index, int del_with_shelf) 
{
    int found_del = 0;
    delete_happened = 1;
    total_b_count--;
    for (unsigned int i = 0; i < shelf->size; i++) {
        if ((*(shelf->books_p + i))->real_ind > -1 && (!wcscmp(index, (*(shelf->books_p + i))->real_ind_w)) && !found_del) {
            found_del = 1;
            if(!del_with_shelf) {
                stfl_modify(form, (*(shelf->books_p + i))->init_ind_w, L"delete", 0);
            }
            (*(shelf->books_p + i))->real_ind = -1;
            delete_book_db((*(shelf->books_p + i))->db_ind);
        } else if ((*(shelf->books_p + i))->real_ind > -1 && found_del) {
            (*(shelf->books_p + i))->real_ind--;
            swprintf((*(shelf->books_p + i))->real_ind_w, 10, L"%d",  (*(shelf->books_p + i))->real_ind);
        }
    }
}

void purge_books(struct stfl_form* form, struct b_entries* shelf) {
    for(int i = 0; i < shelf->size; i++) {
        if ((*(shelf->books_p + i))->real_ind > -1) {
            if ((*(shelf->books_p + i))->real_ind != (*(shelf->books_p + i))->init_ind) {
                stfl_modify(form, (*(shelf->books_p + i))->init_ind_w, L"delete", 0);
                swprintf((*(shelf->books_p + i))->init_ind_w, 10, L"%d", (*(shelf->books_p + i))->real_ind);
            } else {
                stfl_modify(form, (*(shelf->books_p + i))->real_ind_w, L"delete", 0);
            }
            
        }
    }
}

static int digits_increase(int size) {
    int digits_count = size;
    int digits_increase = 0;
    while(digits_count / 10) {
        digits_increase++;
        digits_count /= 10;
    }
    return SHELF_NAME_S + digits_increase;
}
