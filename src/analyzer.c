#include "analyzer.h"

/**
 * @brief Finds a terminal identifier by terminal name.
 * @param g Parsed grammar.
 * @param name Terminal name to search.
 * @return Terminal id if found, otherwise -1.
 */
static int find_terminal_id(const grammar *g, const char *name)
{
	 if (g == NULL || name == NULL || g->terminals == NULL || g->num_terminals <= 0)
    {
        return -1;
    }

    for (int i = 0; i < g->num_terminals; i++)
    {
        if (g->terminals[i].symbol != NULL &&
            strcmp(g->terminals[i].symbol, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Appends one symbol to a dynamically sized symbol array.
 * @param arr Target array pointer.
 * @param count Current element count; incremented on success.
 * @param text Symbol text to copy.
 * @param is_terminal Indicates terminal/non-terminal role.
 * @return true on success, false on allocation failure.
 */
static bool add_symbol_to_array(symbol **arr, int *count, const char *text, bool is_terminal)
{
	if (arr == NULL || count == NULL || text == NULL)
    {
        return false;
    }

    int old_count = *count;
    int new_count = old_count + 1;

    // Allocate or grow the array
    symbol *resized = (symbol *)realloc(*arr, (size_t)new_count * sizeof(symbol));
    if (resized == NULL)
    {
        return false;
    }

    *arr = resized;

    // Duplicate the string for this symbol
    char *dup_text = strdup(text);
    if (dup_text == NULL)
    {
        // We don't increase count because initialization failed
        return false;
    }

    (*arr)[old_count].symbol = dup_text;
    (*arr)[old_count].symbol_length = (int)strlen(dup_text);
    (*arr)[old_count].is_terminal = is_terminal;

    *count = new_count;

    return true;
}

/**
 * @brief Builds FIRST and nullable tables for all non-terminals.
 * @param g Parsed grammar.
 * @param first_table Output flattened table: non-terminal x terminal.
 * @param nullable Output nullable flags per non-terminal.
 * @param epsilon_id Output id of terminal "epsilon", or -1 if absent.
 * @return true when tables were built, false on invalid input or allocation error.
 */
static bool compute_first_tables(const grammar *g, bool **first_table, bool **nullable, int *epsilon_id)
{
	// TODO: Allocate FIRST/nullable tables and compute them with fixed-point propagation over productions.
	if (g == NULL || first_table == NULL || nullable == NULL || epsilon_id == NULL)
    {
        return false;
    }

    int N = g->num_non_terminals;
    int T = g->num_terminals;

    if (N <= 0 || T <= 0 || g->productions == NULL || g->num_productions <= 0)
    {
        return false;
    }

    // Allocate FIRST and nullable tables
    bool *ft = (bool *)calloc((size_t)N* (size_t)T, sizeof(bool));
    bool *nul = (bool *)calloc((size_t)N, sizeof(bool));
    if (ft == NULL || nul == NULL)
    {
        free(ft);
        free(nul);
        return false;
    }

    *first_table = ft;
    *nullable = nul;

    // Find the terminal "epsilon"
    int eps_id = find_terminal_id(g, "epsilon");
    *epsilon_id = eps_id;

    bool changed = true;

    // ----- FIXED POINT: repeat until no changes -----
    while (changed)
    {
        changed = false;

        for (int p_i = 0; p_i < g->num_productions; p_i++)
        {
            production p = &g->productions[p_i];
            int A = p->non_terminal_id;

            if (A < 0 || A >= N)
                continue;

            if (p->production_length == 0)
            {
                // A -> (empty)
                if (!nul[A])
                {
                    nul[A] = true;
                    changed = true;
                }
                continue;
            }

            bool all_nullable = true;

            for (int j = 0; j < p->production_length; j++)
            {
                int sym_id = p->production_symbol_ids[j];

                // Case 1: sym is a terminal
                if (sym_id >= 0 && sym_id < T)
                {
                    if (sym_id != eps_id) // ignore epsilon as terminal FIRST element
                    {
                        size_t idx = (size_t)A* (size_t)T + (size_t)sym_id;
                        if (!ft[idx])
                        {
                            ft[idx] = true;
                            changed = true;
                        }
                    }

                    // terminal stops the chain
                    all_nullable = false;
                    break;
                }
                // Case 2: sym is a non-terminal B
                else if (sym_id >= T && sym_id < T + N)
                {
                    int B = sym_id - T;

                    // FIRST(B) \ {epsilon} ⊆ FIRST(A)
                    for (int t = 0; t < T; t++)
                    {
                        if (t == eps_id)
                            continue;

                        size_t idxB = (size_t)B* (size_t)T + (size_t)t;
                        if (ft[idxB])
                        {
                            size_t idxA = (size_t)A* (size_t)T + (size_t)t;
                            if (!ft[idxA])
                            {
                                ft[idxA] = true;
                                changed = true;
                            }
                        }
                    }

                    // If B is NOT nullable → stop
                    if (!nul[B])
                    {
                        all_nullable = false;
                        break;
                    }

                    // else keep scanning RHS
                }
                else
                {
                    // invalid index → cannot treat as nullable safely
                    all_nullable = false;
                    break;
                }
            }

            // If all RHS symbols nullable → A is nullable
            if (all_nullable)
            {
                if (!nul[A])
                {
                    nul[A] = true;
                    changed = true;
                }
            }
        }
    }

    return true;
}

/**
 * @brief Builds FOLLOW table for all non-terminals.
 * @param g Parsed grammar.
 * @param first_table FIRST table from compute_first_tables.
 * @param nullable Nullable flags from compute_first_tables.
 * @param epsilon_id Terminal id for "epsilon", or -1.
 * @param out_follow Output flattened table: non-terminal x (terminals + '$').
 * @param out_follow_cols Output number of columns for out_follow.
 * @return true on success, false on allocation error or invalid input.
 */
static bool compute_follow_table(
	const grammar *g,
	const bool *first_table,
	const bool *nullable,
	int epsilon_id,
	bool **out_follow,
	int *out_follow_cols)
{
	// TODO: Allocate FOLLOW structures and propagate FOLLOW sets right-to-left until convergence.
	if (g == NULL || first_table == NULL || nullable == NULL ||
        out_follow == NULL || out_follow_cols == NULL)
    {
        return false;
    }

    int N = g->num_non_terminals;
    int T = g->num_terminals;
    int P = g->num_productions;

    if (N <= 0 || T <= 0 || P <= 0 || g->productions == NULL)
    {
        return false;
    }
	 int cols = T + 1;
    bool *follow = (bool *)calloc((size_t)N* (size_t)cols, sizeof(bool));
    if (follow == NULL)
    {
        return false;
    }

    *out_follow = follow;
    *out_follow_cols = cols;

    // Rule 1: FOLLOW(start symbol) contains '$'
	follow[0 * cols + T] = true;

    bool changed = true;

    // Fixed-point iteration
    while (changed)
    {
        changed = false;

        for (int p_i = 0; p_i < P; p_i++)
        {
            production p = &g->productions[p_i];
            int A = p->non_terminal_id;

            if (A < 0 || A >= N)
                continue;

            int len = p->production_length;

            // Scan RHS: A → X1 X2 ... Xn
            for (int i = 0; i < len; i++)
            {
                int sym_id = p->production_symbol_ids[i];

                // We only process non-terminals on the RHS
                if (sym_id < T || sym_id >= T + N)
                    continue;

                int B = sym_id - T;

                // β = suffix after B
                bool beta_nullable = true;

                for (int j = i + 1; j < len; j++)
                {
                    int sym_j = p->production_symbol_ids[j];

                    // Case 1: β[j] is a terminal
                    if (sym_j >= 0 && sym_j < T)
                    {
                        if (sym_j != epsilon_id)
                        {
                            size_t idx = (size_t)B * (size_t)cols + (size_t)sym_j;
                            if (!follow[idx])
                            {
                                follow[idx] = true;
                                changed = true;
                            }
                        }
                        beta_nullable = (sym_j == epsilon_id);
                        if (!beta_nullable)
                            break;
                    }
                    // Case 2: β[j] is a non-terminal C
                    else if (sym_j >= T && sym_j < T + N)
                    {
                        int C = sym_j - T;

                        // Add FIRST(C) \ {ε} into FOLLOW(B)
                        for (int t = 0; t < T; t++)
                        {
                            if (t == epsilon_id)
                                continue;

                            size_t idxC = (size_t)C * (size_t)T + (size_t)t;
                            if (first_table[idxC])
                            {
                                size_t idxB = (size_t)B * (size_t)cols + (size_t)t;
                                if (!follow[idxB])
                                {
                                    follow[idxB] = true;
                                    changed = true;
                                }
                            }
                        }

                        if (!nullable[C])
                        {
                            beta_nullable = false;
                            break;
                        }
                    }
                    else
                    {
                        beta_nullable = false;
                        break;
                    }
                }

                // Case 3: If β is nullable, FOLLOW(A) ⊆ FOLLOW(B)
                if (beta_nullable)
                {
                    for (int c = 0; c < cols; c++)
                    {
                        size_t idxA = (size_t)A * (size_t)cols + (size_t)c;
                        size_t idxB = (size_t)B * (size_t)cols + (size_t)c;

                        if (follow[idxA] && !follow[idxB])
                        {
                            follow[idxB] = true;
                            changed = true;
                        }
                    }
                }
            }
        }
    }

    return true;
}

/**
 * @brief Collects FIRST symbols for one non-terminal from the computed table.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index.
 * @param first_table FIRST table.
 * @param nullable Nullable flags.
 * @param epsilon_id Terminal id for "epsilon", or -1.
 * @param out_first Output array with FIRST symbols.
 * @return Number of collected symbols, or 0 on error.
 */
static int collect_first_for_non_terminal(
    const grammar *g,
    int non_terminal_id,
    const bool *first_table,
    const bool *nullable,
    int epsilon_id,
    symbol **out_first)
{
    if (g == NULL || first_table == NULL || nullable == NULL ||
        out_first == NULL ||
        non_terminal_id < 0 || non_terminal_id >= g->num_non_terminals)
    {
        return 0;
    }
 
    int T = g->num_terminals;
    symbol *result = NULL;
    int count = 0;
 
    // FIRST(A) contains every terminal t where first_table[A][t] is true
    for (int t = 0; t < T; t++)
    {
        if (t == epsilon_id)
            continue; // do not add epsilon here yet
 
        size_t idx = (size_t)non_terminal_id * (size_t)T + (size_t)t;
 
        if (first_table[idx])
        {
            add_symbol_to_array(&result, &count, g->terminals[t].symbol, true);
        }
    }
 
    // If nullable, add epsilon explicitly
    if (nullable[non_terminal_id] && epsilon_id >= 0)
    {
        add_symbol_to_array(&result, &count, g->terminals[epsilon_id].symbol, true);
    }
 
    *out_first = result;
    return count;
}
 

/**
 * @brief Collects FOLLOW symbols for one non-terminal from the computed table.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index.
 * @param follow_table FOLLOW table.
 * @param follow_cols Number of columns in follow_table.
 * @param out_follow Output array with FOLLOW symbols.
 * @return Number of collected symbols, or 0 on error.
 */
static int collect_follow_for_non_terminal(
    const grammar *g,
    int non_terminal_id,
    const bool *follow_table,
    int follow_cols,
    symbol **out_follow)
{
    if (g == NULL || follow_table == NULL || out_follow == NULL ||
        non_terminal_id < 0 || non_terminal_id >= g->num_non_terminals ||
        follow_cols <= 0)
    {
        return 0;
    }
 
    int T = g->num_terminals;
    int cols = follow_cols;
 
    symbol *result = NULL;
    int count = 0;
 
    // 1. Terminals in positions 0..T-1
    for (int t = 0; t < T; t++)
    {
        size_t idx = (size_t)non_terminal_id * (size_t)cols + (size_t)t;
 
        if (follow_table[idx])
        {
            // Add the terminal's symbol text
            add_symbol_to_array(&result, &count, g->terminals[t].symbol, true);
        }
    }
 
    // 2. End-of-input symbol '$' is stored in column T
    if (T < cols)
    {
        size_t idx_dollar = (size_t)non_terminal_id * (size_t)cols + (size_t)T;
 
        if (follow_table[idx_dollar])
        {
            add_symbol_to_array(&result, &count, "$", true);
        }
    }
 
    *out_follow = result;
    return count;
}
 

/**
 * @brief Computes FIRST set for one non-terminal by index.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index in g->non_terminals.
 * @param out_first Output array with FIRST symbols.
 * @return Number of symbols in out_first, or 0 on error.
 */
int compute_first_for_non_terminal(const grammar *g, int non_terminal_id, symbol **out_first)
{
	// TODO: Validate inputs, compute shared FIRST tables, and collect FIRST for the requested non-terminal.
	if (g == NULL || out_first == NULL ||
        non_terminal_id < 0 || non_terminal_id >= g->num_non_terminals)
    {
        return 0;
    }

    bool *first_table = NULL;
    bool *nullable = NULL;
    int epsilon_id = -1;

    // Build full FIRST + nullable tables
    if (!compute_first_tables(g, &first_table, &nullable, &epsilon_id))
    {
        return 0;
    }

    // Extract FIRST(non_terminal_id)
    int count = collect_first_for_non_terminal(
        g,
        non_terminal_id,
        first_table,
        nullable,
        epsilon_id,
        out_first);

    // Free temporary tables
    free(first_table);
    free(nullable);

    return count;
}

/**
 * @brief Computes FOLLOW set for one non-terminal by index.
 * @param g Parsed grammar.
 * @param non_terminal_id Non-terminal index in g->non_terminals.
 * @param out_follow Output array with FOLLOW symbols.
 * @return Number of symbols in out_follow, or 0 on error.
 */
int compute_follow_for_non_terminal(const grammar *g, int non_terminal_id, symbol **out_follow)
{
	// TODO: Validate inputs, compute FIRST/nullable and FOLLOW tables, then collect FOLLOW for the target non-terminal.
	if (g == NULL || out_follow == NULL ||
        non_terminal_id < 0 || non_terminal_id >= g->num_non_terminals)
    {
        return 0;
    }

    bool *first_table = NULL;
    bool *nullable = NULL;
    int epsilon_id = -1;

    // STEP 1: Build FIRST and nullable tables
    if (!compute_first_tables(g, &first_table, &nullable, &epsilon_id))
    {
        return 0;
    }

    // STEP 2: Build FOLLOW table
    bool *follow_table = NULL;
    int follow_cols = 0;

    if (!compute_follow_table(g,
                              first_table,
                              nullable,
                              epsilon_id,
                              &follow_table,
                              &follow_cols))
    {
        free(first_table);
        free(nullable);
        return 0;
    }

    // STEP 3: Extract FOLLOW(non_terminal_id)
    int count = collect_follow_for_non_terminal(
        g,
        non_terminal_id,
        follow_table,
        follow_cols,
        out_follow);

    // STEP 4: Free temporary structures
    free(first_table);
    free(nullable);
    free(follow_table);

    return count;
	
}

/**
 * @brief Computes FIRST set for the start symbol.
 * @param g Parsed grammar.
 * @param out_first Output array with FIRST(start) terminals.
 * @return Number of symbols in out_first, or 0 on error.
 */
int compute_first_for_start_symbol(const grammar *g, symbol **out_first)
{
    if (g == NULL || out_first == NULL || g->num_non_terminals <= 0)
    {
        return 0;
    }
    // By convention, the first non-terminal is the start symbol
    return compute_first_for_non_terminal(g, 0, out_first);
}
 
/**
 * @brief Computes FOLLOW set for the start symbol.
 * @param g Parsed grammar.
 * @param out_follow Output array with FOLLOW(start) terminals.
 * @return Number of symbols in out_follow, or 0 on error.
 */
 
int compute_follow_for_start_symbol(const grammar *g, symbol **out_follow)
{
    if (g == NULL || out_follow == NULL || g->num_non_terminals <= 0)
    {
        return 0;
    }
 
    // By convention, the first non-terminal is the start symbol
    return compute_follow_for_non_terminal(g, 0, out_follow);
}

/**
 * @brief Frees a symbol array and each duplicated symbol string.
 * @param symbols Symbol array to release.
 * @param count Number of initialized entries.
 * @return This function does not return a value.
 */
void free_symbol_array(symbol *symbols, int count)
{
    if (symbols == NULL || count <= 0)
    {
        return;
    }

    for (int i = 0; i < count; i++)
    {
        free(symbols[i].symbol);
        symbols[i].symbol = NULL;
    }

    free(symbols);
}
