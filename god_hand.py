import pandas
def touch_of_midas(string: str):
    final_string = string.replace("\n", "").replace("\r", "").split(" ")
    try:
        while True:
            final_string.remove('')
    except ValueError:
        pass
    return " ".join(final_string)


CSV_PATH = "spotify_millsongdata.csv"

df = pandas.read_csv(CSV_PATH)

df["text"] = df["text"].apply(touch_of_midas)
df = df.drop(columns=["link"])

df.to_csv("golden_music.csv",index=False, sep="|")