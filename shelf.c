#include "shelf.h"

static const wchar_t* shelves_header = L"Your shelves";
static const wchar_t* books_header = L"Your books";
static const wchar_t* shelves_hints = L"ENTER:Select x:eXit a:Add book e:Edit shelf d:Delete shelf";
static const wchar_t* shelves_hints_start = L"ENTER:Select x:eXit a:Add book";
static const wchar_t* books_hints = L"q:Return x:eXit a:Add book e:Edit d:Delete book";
static const wchar_t* books_hints_start = L"q:Return x:eXit a:Add book";
static const wchar_t* edit_hints = L"ENTER:Confirm ESC:Discard";
unsigned int w_size;
static uint32_t shelves_loop(struct stfl_form*, struct b_entries*);
static uint32_t books_loop(struct stfl_form*, struct b_entries*);
static uint32_t (*loop_func[2])(struct stfl_form*, struct b_entries*) = { shelves_loop, books_loop };
static void edit_sm(struct stfl_form* form, struct b_entries* shelf);
static void edit_shelf_sm(struct stfl_form* form, struct b_entries* shelf);
static void toggle_edit(struct stfl_form* form);
static void init_books_info(struct stfl_form* form);
static void init_shelves_info(struct stfl_form* form);
struct b_entries* all_books;
static int edit_show;
static int edit_shelf;
static int edit_mode;
static int sm_state;
static wchar_t* new_author; 
static wchar_t* new_book;
static wchar_t* new_shelf;
static wchar_t* entry_edit;
int delete_happened;

static uint32_t shelves_loop(struct stfl_form* form, struct b_entries* shelf)
{
    uint32_t ret_val = 0;
    const wchar_t* event = 0;
    if(!populate_shelves(form, shelf)) {
        ret_val = EXIT;
        goto EXIT_SHELVES;
    }
    init_shelves_info(form);
    for (;;) {
        if (!edit_show) {
            stfl_set_focus(form, L"shelves");
        } else {
            stfl_set_focus(form, L"in_field");
        }
        event = stfl_run(form, 0);
        if (event) {
            if (!wcscmp(L"shelves", stfl_get_focus(form)) && !edit_show) {
                if (!wcscmp(event, L"x") || !wcscmp(event, L"X")) {
                    ret_val = EXIT;
                    break;
                } else if (!wcscmp(event, L"ENTER")) {
                    const wchar_t* b_shelf = stfl_get(form, L"itemposs");
                    struct b_entries* shelves = all_books;
                    while (shelves != 0) {
                        if(!wcscmp(b_shelf, shelves->real_ind_w)) {
                            ret_val = (uint32_t) shelves-> init_ind << 16;
                            break;
                        }
                        shelves = shelves->next;
                    }
                    ret_val |= BOOKS;
                    break;
                } else if (!wcscmp(event, L"a") || !wcscmp(event, L"A")) {
                    edit_mode = BOOK_ADD_MODE_SHELF;
                    edit_sm(form, shelf);
                } else if (!wcscmp(event, L"e") || !wcscmp(event, L"E")) {
                    const wchar_t* b_shelf = stfl_get(form, L"itemposs");
                    struct b_entries* shelves = all_books;
                    while (shelves != 0) {
                        if(!wcscmp(b_shelf, shelves->real_ind_w)) {
                            edit_shelf_sm(form, shelves);
                            break;
                        }
                        shelves = shelves->next;
                    }
                } else if (!wcscmp(event, L"d") || !wcscmp(event, L"D")) {
                    const wchar_t* b_shelf = stfl_get(form, L"itemposs");
                    struct b_entries* shelves = all_books;
                    while (shelves != 0) {
                        if(!wcscmp(b_shelf, shelves->real_ind_w)) {
                            delete_shelf(form, shelves, shelves->shelf); 
                            break;
                        }
                        shelves = shelves->next;
                    }
                }
            } else if (edit_show && (wcscmp(L"in_field", stfl_get_focus(form)) || wcscmp(L"in_field_text", stfl_get_focus(form)))) {
                if (!wcscmp(event, L"ENTER")) {
                    if (edit_shelf) {
                        edit_shelf_sm(form, shelf);
                    } else {
                        edit_sm(form, shelf);
                    }
                } else if (!wcscmp(event, L"ESC")) {
                    sm_state = STATE_START;
                    toggle_edit(form);
                    edit_shelf = 0;
                    init_shelves_info(form);
                }
            }
        }
    }
    purge_shelves(form, shelf);
    EXIT_SHELVES:
    return ret_val;
}

static uint32_t books_loop(struct stfl_form* form, struct b_entries* shelf)
{
    uint32_t ret_val = 0;
    const wchar_t* event = 0;
    edit_show = 0;
    if(!populate_books(form, shelf)) {
        ret_val = EXIT;
        goto EXIT_BOOKS;
    }
    init_books_info(form);
    for (;;) {
        if (!edit_show) {
            stfl_set_focus(form, L"books");
        } else {
            stfl_set_focus(form, L"in_field");
        }
        event = stfl_run(form, 0);
        if (event) {
            if (!wcscmp(L"books", stfl_get_focus(form)) && !edit_show) {
                if (!wcscmp(event, L"x")) {
                    ret_val = EXIT;
                    break;
                } else if (!wcscmp(event, L"a") || !wcscmp(event, L"A")) {
                    edit_mode = BOOK_ADD_MODE;
                    edit_sm(form, shelf);
                } else if (!wcscmp(event, L"q") || !wcscmp(event, L"Q")) {
                    ret_val = SHELVES;
                    break;
                } else if (!wcscmp(event, L"e") || !wcscmp(event, L"E")) {
                    edit_mode = BOOK_EDIT_MODE;
                    int edit_size = wcslen(stfl_get(form, L"itemposb")) + 1;
                    entry_edit = malloc(edit_size * w_size);
                    wmemcpy(entry_edit, stfl_get(form, L"itemposb"), edit_size);
                    edit_sm(form, shelf);
                } else if (!wcscmp(event, L"d") || !wcscmp(event, L"D")) {
                    const wchar_t* to_del = stfl_get(form, L"itemposb");
                    delete_book(form, shelf, to_del, 0);
                } 
            } else if (edit_show && (wcscmp(L"in_field", stfl_get_focus(form)) || wcscmp(L"in_field_text", stfl_get_focus(form)))) {
                if (!wcscmp(event, L"ENTER")) {
                    edit_sm(form, shelf);
                } else if (!wcscmp(event, L"ESC")) {
                    sm_state = STATE_START;
                    toggle_edit(form);
                    init_books_info(form);
                }
            }
        }
    }
    purge_books(form, shelf);
    EXIT_BOOKS:
    return ret_val;
}

static void init_books_info(struct stfl_form* form)
{
    if(book_stub) {
        init_shelves(form, books_header, books_hints_start);
    } else if (!book_stub) {
        init_shelves(form, books_header, books_hints);
    }
}

static void init_shelves_info(struct stfl_form* form)
{
    if(shelf_stub) {
        init_shelves(form, shelves_header, shelves_hints_start);
    } else {
        init_shelves(form, shelves_header, shelves_hints);
    }
}

static void edit_sm(struct stfl_form* form, struct b_entries* shelf)
{
    static int book_size;
    static int author_size;
    static int shelf_size;
    static int edit_book_index;
    static int entry_changed;
    switch (sm_state) {
        case STATE_START:
            toggle_edit(form);
            stfl_set(form, L"inp_hint", L"Book name: ");
            sm_state = STATE_NAME;
            entry_changed = 0;
            edit_book_index = 0;
            if (edit_mode == BOOK_EDIT_MODE) {
                const wchar_t* book_ind = stfl_get(form, L"itemposb");
                for (int i = 0; i < shelf->size; i++) {
                    if (!wcscmp(book_ind, (*(shelf->books_p + i))->real_ind_w)) {
                        edit_book_index = i;
                        break;
                    } 
                }
                stfl_set(form, L"in_field_text", (*(shelf->books_p + edit_book_index))->book);
            }
            break;
        case STATE_NAME:
            book_size = wcslen(stfl_get(form, L"in_field_text"));
            if (book_size == 0) {
                return;
            }
            book_size = book_size + 1;
            new_book = malloc(book_size * w_size);
            wmemcpy(new_book, stfl_get(form, L"in_field_text"), book_size);
            stfl_set(form, L"inp_hint", L"Author: ");
            if (edit_mode == BOOK_EDIT_MODE) {
                if (wcscmp((*(shelf->books_p + edit_book_index))->book, stfl_get(form, L"in_field_text"))) {
                    entry_changed = 1;
                }
                stfl_set(form, L"in_field_text", (*(shelf->books_p + edit_book_index))->author);
            } else {
                stfl_set(form, L"in_field_text", L"");
            }
            sm_state = STATE_AUTHOR;
            break;
        case STATE_AUTHOR:
            author_size = wcslen(stfl_get(form, L"in_field_text"));
            if (author_size == 0) {
                return;
            }
            author_size = author_size + 1;
            if (edit_mode == BOOK_EDIT_MODE && 
                    wcscmp((*(shelf->books_p + edit_book_index))->author, stfl_get(form, L"in_field_text"))) {
                entry_changed = 1;  
            }
            new_author = malloc(author_size * w_size);
            wmemcpy(new_author, stfl_get(form, L"in_field_text"), author_size);
            stfl_set(form, L"inp_hint", L"Shelf: ");
            if (edit_mode != BOOK_ADD_MODE_SHELF) {
                stfl_set(form, L"in_field_text", shelf->shelf);
            } else {
                stfl_set(form, L"in_field_text", L"");
            }
            sm_state = STATE_SHELF;
            break;
        case STATE_SHELF:
            shelf_size = wcslen(stfl_get(form, L"in_field_text"));
            if (shelf_size == 0) {
                return;
            }
            shelf_size = shelf_size + 1;
            new_shelf = malloc(shelf_size * w_size);
            wmemcpy(new_shelf, stfl_get(form, L"in_field_text"), shelf_size);
            stfl_set(form, L"in_field_text", L"");
            sm_state = STATE_START;
            toggle_edit(form);
            if (edit_mode == BOOK_EDIT_MODE) {
                if (!wcscmp(shelf->shelf, new_shelf) && !entry_changed) {
                    edit_book_index = 0;
                    free(entry_edit);
                    free(new_book);
                    free(new_author);
                    free(new_shelf);
                    break;
                }
                delete_book(form, shelf, entry_edit, 0);
                free(entry_edit);
            }
            struct b_entries* last_entry;
            struct b_entries* temp_shelf = all_books;
            int shelf_exists = 0;
            while (temp_shelf != NULL) {
                if (temp_shelf->shelf != NULL && !wcscmp(temp_shelf->shelf, new_shelf)) {
                    shelf_exists = 1;
                    break;
                }
                last_entry = temp_shelf;
                temp_shelf = temp_shelf->next;
            }
            if (shelf_exists) {
                if (!add_book_db(temp_shelf, new_author, new_book)) {
                    return;
                }
                load_book_mem(temp_shelf, temp_shelf->size, author_size * w_size,
                    book_size * w_size, total_b_count, new_author, new_book);
                if (!wcscmp(new_shelf, shelf->shelf) && edit_mode != BOOK_ADD_MODE_SHELF) {
                    add_book(form, shelf, new_author, new_book);
                }
            } else {
                
                if (all_books == last_entry && last_entry->shelf == NULL) {
                    last_entry->shelf = new_shelf;
                    last_entry->init_ind = 0;
                    last_entry->real_ind = 0;
                    swprintf(last_entry->init_ind_w, 10, L"%d", last_entry->init_ind);
                    swprintf(last_entry->real_ind_w, 10, L"%d", last_entry->real_ind);
                    if (!add_book_db(last_entry, new_author, new_book)) {
                        return;
                    }
                    load_book_mem(last_entry, last_entry->size, author_size * w_size,
                    book_size * w_size, total_b_count, new_author, new_book);
                    add_shelf(form, last_entry);
                    stfl_modify(form, L"-1", L"delete", 0);
                } else {
                    struct b_entries* next_shelf = init_data();
                    next_shelf->shelf = new_shelf;
                    next_shelf->init_ind = last_entry->init_ind + 1;
                    next_shelf->real_ind = last_entry->real_ind + 1;
                    swprintf(next_shelf->init_ind_w, 10, L"%d", next_shelf->init_ind);
                    swprintf(next_shelf->real_ind_w, 10, L"%d", next_shelf->real_ind);
                    last_entry->next = next_shelf;
                    if (!add_book_db(next_shelf, new_author, new_book)) {
                        return;
                    }
                    load_book_mem(next_shelf, next_shelf->size, author_size * w_size,
                    book_size * w_size, total_b_count, new_author, new_book);
                    if (edit_mode == BOOK_ADD_MODE_SHELF) {
                        add_shelf(form, next_shelf);
                    }
                }
            }
            break;
        default:
            break;
    }
}

static void edit_shelf_sm(struct stfl_form* form, struct b_entries* shelf)
{
    static int shelf_size;
    static struct b_entries* temp_shelf;
    switch (sm_state) {
        case STATE_START:
            toggle_edit(form);
            edit_shelf = 1;
            temp_shelf = shelf;
            stfl_set(form, L"inp_hint", L"Shelf: ");
            stfl_set(form, L"in_field_text", (shelf->shelf));
            sm_state = STATE_SHELF;
            break;
        case STATE_SHELF:
            shelf_size = wcslen(stfl_get(form, L"in_field_text"));
            if (shelf_size == 0) {
                return;
            }
            shelf_size = shelf_size + 1;
            if (!wcscmp(temp_shelf->shelf, stfl_get(form, L"in_field_text"))) {
                goto STATE_SHELF_END;
            }
            modify_shelf_db(stfl_get(form, L"in_field_text"), temp_shelf->shelf);
            free(temp_shelf->shelf);
            temp_shelf->shelf = malloc(shelf_size * w_size);
            wmemcpy(temp_shelf->shelf, stfl_get(form, L"in_field_text"), shelf_size);
            modify_shelf(form, temp_shelf, temp_shelf->real_ind_w);
            STATE_SHELF_END:
            toggle_edit(form);
            edit_shelf = 0;
            sm_state = STATE_START;
            init_shelves_info(form);
            break;
        default:
            break;
    }
}

static void toggle_edit(struct stfl_form* form)
{
    if (edit_show) {
        stfl_set(form, L"showedit", L"0");
        edit_show = 0;
    } else {
        stfl_set(form, L"showedit", L"1");
        stfl_set(form, L"help", edit_hints);
        edit_show = 1;
    }
}

int main(int argc, char** argv)
{
    uint16_t shelf_to = 0;
    uint32_t loop_res = 0;
    struct b_entries* next_shelf;
    w_size = sizeof(wchar_t);
    edit_show = 0;
    edit_shelf = 0;
    edit_mode = 0;
    delete_happened = 0;
    sm_state = STATE_START;
    all_books = init_data();
    if (!init_db())
        goto EXIT_ON_ERROR;

    uint32_t exec_func = SHELVES;
    struct stfl_form* s = stfl_create(L"<shelves.stfl>");
    struct stfl_form* b = stfl_create(L"<books.stfl>");
    for (;;) {
        switch (exec_func) {
        case SHELVES:
            loop_res = loop_func[SHELVES](s, all_books);
            exec_func = 0xFFFF & loop_res;
            shelf_to = loop_res >> 16;
            break;
        case BOOKS:
            next_shelf = all_books;
            for (uint16_t i = 0; i < shelf_to; i++) {
                next_shelf = next_shelf->next;
            }
            exec_func = loop_func[BOOKS](b, next_shelf);
            break;
        case EXIT:
            goto EXIT_PROGRAM;
        }
    }
EXIT_PROGRAM:
    stfl_reset();
    stfl_free(s);
    stfl_free(b);
    close_db();
    return 0;
EXIT_ON_ERROR:
    return 1;
}
