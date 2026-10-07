import pandas as pd
from collections import Counter


def get_nth_table(grid, first_row_is_header, title):
    if first_row_is_header:
        header = grid[0]
        table = grid[1:]
        df = pd.DataFrame(table, columns=header)
    else:
        df = pd.DataFrame(grid)

    df.to_csv(f"{title}.csv", encoding="utf-8")

    print(df)

    df_without_headers = df.set_index(df.columns[0], inplace=False)
    return count_elements_in_table(df_without_headers)


def count_elements_in_table(table):
    # Pobieramy wszystkie wartości z wyjątkiem nagłówków (flatten spłaszcza żebyśmy mieli wszystko w jendnym wymiarze, liście)
    table = table.values.flatten()
    all_values = [v for v in table if v]

    values_count = Counter(all_values)
    count_df = pd.DataFrame(values_count.items(), columns=["value", "count"])
    count_df = count_df.sort_values("count", ascending=False).reset_index(drop=True)  # Sortujemy od największego
    return count_df
