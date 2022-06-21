from fnmatch import translate
import requests, json, urllib.parse

content = []
for i in range(250):
    r = requests.get(f'http://public-api.unijzlsx.com/getalias/{i}')
    print(r.text)
    if r.text != '{"status":"false","message":"musicid not found"}':
        entry = {}
        entry["musicId"] = i
        entry["alias"] = r.text.encode('raw_unicode_escape').decode('unicode_escape').encode('utf-16', 'surrogatepass').decode('utf-16').split('，')
        r = json.loads(requests.get(f'http://public-api.unijzlsx.com/getsongid/{urllib.parse.quote_plus(entry["alias"][0])}').text)
        assert(i == r["musicId"])
        entry["title"] = r["title"]
        entry["translate"] = r["translate"]
        content.append(entry)
with open("a.txt", "w", encoding='utf-8') as file:
    file.write(json.dumps(content, ensure_ascii=False))
