def summarize(data):
    text = data.find("p")
    return text.get_text().replace("\\n", "\n").strip()
