#include <stdio.h>
#include "st.h"

int main(void)
{
  char *keys[] = {"a", "b", "c", "d", "e", "f"};
  st_data_t values[] = {1, 2, 3, 4, 5, 6};

  st_table *tab = st_init_strtable();
  if (!tab) return 1;

  for (int i = 0; i < 6; i++)
    st_insert(tab, (st_data_t)keys[i], values[i]);

  st_free_table(tab);
  return 0;
}
