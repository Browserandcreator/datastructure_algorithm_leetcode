#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Leet Manager (robust CN/COM, CSRF-aware)

- Auto CN -> COM fallback (unless --site specified)
- Use requests.Session per-site; warm up to get csrftoken
- Send x-csrftoken + cookies for GraphQL
- Fallback to /api/problems/all/ when GraphQL slug lookup fails
- README now includes:
  * Overview table (same as before)
  * Index by Difficulty (Easy/Medium/Hard)
  * Index by Topic (Data Structure / topicTags)

Commands: new / quick / remove / index / readme
"""

import argparse
import csv
import datetime as dt
import json
import re
import shutil
import sys
from pathlib import Path

try:
    import requests
except Exception:
    requests = None

# ---------------------- Paths ----------------------

THIS_FILE = Path(__file__).resolve()
if THIS_FILE.parent.name.lower() in ("script", "scripts"):
    ROOT = THIS_FILE.parent.parent
else:
    ROOT = Path.cwd()

PROBLEMS = ROOT / "problems"
PROBLEMS.mkdir(parents=True, exist_ok=True)
INDEX_CSV = ROOT / "INDEX.csv"
README_MD = ROOT / "README.md"

# ---------------------- Helpers ----------------------

def _site_base(site: str) -> str:
    return "https://leetcode.cn" if site == "cn" else "https://leetcode.com"

def _graphql_url(site: str) -> str:
    return _site_base(site) + "/graphql/"

def _ensure_dir(p: Path):
    p.parent.mkdir(parents=True, exist_ok=True)

def write_file(path: Path, content: str, encoding: str = "utf-8"):
    _ensure_dir(path)
    with open(path, "w", encoding=encoding, newline="") as f:
        f.write(content)

def zero4(n: int) -> str:
    return f"{int(n):04d}"

def sanitize_dir_name(qid: int, slug: str) -> str:
    slug = re.sub(r"[^a-z0-9\\-]", "-", (slug or "").lower())
    slug = re.sub(r"-{2,}", "-", slug).strip("-")
    return f"{zero4(qid)}-{slug}" if slug else zero4(qid)

def html_to_text(s: str) -> str:
    if not s:
        return ""
    s = re.sub(r"<\\s*br\\s*/?>", "\\n", s, flags=re.I)
    s = re.sub(r"</\\s*p\\s*>", "\\n\\n", s, flags=re.I)
    s = re.sub(r"<\\s*li\\s*>", "- ", s, flags=re.I)
    s = re.sub(r"<\\s*code\\s*>(.*?)</\\s*code\\s*>", r"`\\1`", s, flags=re.I | re.S)
    s = re.sub(r"<[^>]+>", "", s)
    s = re.sub(r"\\r?\\n\\s*\\r?\\n\\s*\\r?\\n+", "\\n\\n", s)
    return s.strip()

# ---------------------- Session / CSRF ----------------------

class SiteClient:
    """
    Per-site client with session + csrftoken handling.
    """
    def __init__(self, site: str):
        self.site = site  # "cn" or "com"
        self.base = _site_base(site)
        self.sess = requests.Session()
        self.csrf = None

    def _ua_headers(self) -> dict:
        return {
            "User-Agent": "Mozilla/5.0",
            "Accept": "application/json, text/plain, */*",
            "Content-Type": "application/json",
        }

    def warm_up(self):
        try:
            self.sess.get(self.base + "/", timeout=15, headers=self._ua_headers())
            self.sess.get(self.base + "/problemset/all/", timeout=15, headers=self._ua_headers())
        except Exception:
            pass
        self.csrf = self.sess.cookies.get("csrftoken") or self.sess.cookies.get("csrftoken", None)

    def headers(self, referer_path: str = "/") -> dict:
        h = self._ua_headers().copy()
        h["Origin"] = self.base
        h["Referer"] = self.base + referer_path
        if self.csrf:
            h["x-csrftoken"] = self.csrf
        return h

    def post_graphql(self, payload: dict, referer_path: str = "/") -> dict | None:
        url = _graphql_url(self.site)
        try:
            r = self.sess.post(url, json=payload, headers=self.headers(referer_path), timeout=25)
            r.raise_for_status()
            return r.json()
        except Exception:
            return None

    def get_json(self, path: str) -> dict | None:
        try:
            r = self.sess.get(self.base + path, headers=self._ua_headers(), timeout=20)
            r.raise_for_status()
            return r.json()
        except Exception:
            return None

# ---------------------- Fetch (slug/detail) ----------------------

def lookup_slug_by_id_graphql(qid: int, client: SiteClient) -> str | None:
    payload = {
        "operationName": "problemsetQuestionList",
        "variables": {
            "categorySlug": "",
            "skip": 0,
            "limit": 1,
            "filters": {"frontendQuestionId": str(qid)}
        },
        "query": """
        query problemsetQuestionList($categorySlug: String, $limit: Int, $skip: Int, $filters: QuestionListFilterInput) {
          problemsetQuestionList: questionList(categorySlug: $categorySlug, limit: $limit, skip: $skip, filters: $filters) {
            questions: data { titleSlug }
          }
        }"""
    }
    data = client.post_graphql(payload, referer_path="/problemset/all/")
    if not data:
        return None
    qs = (data.get("data", {}) or {}).get("problemsetQuestionList", {}) or {}
    arr = qs.get("questions", []) or []
    return (arr[0] or {}).get("titleSlug")

def lookup_slug_by_id_rest(qid: int, client: SiteClient) -> str | None:
    data = client.get_json("/api/problems/all/")
    if not data:
        return None
    for q in data.get("stat_status_pairs", []):
        stat = q.get("stat") or {}
        fid = stat.get("frontend_question_id")
        slug = stat.get("question__title_slug")
        if str(fid) == str(qid) and slug:
            return slug
    return None

def fetch_question_detail_by_slug(slug: str, client: SiteClient) -> dict:
    payload = {
        "operationName": "questionData",
        "variables": {"titleSlug": slug},
        "query": """
        query questionData($titleSlug: String!) {
          question(titleSlug: $titleSlug) {
            questionId
            questionFrontendId
            title
            titleSlug
            difficulty
            topicTags { name slug }
            translatedTitle
            translatedContent
            content
            codeSnippets { lang langSlug code }
            sampleTestCase
          }
        }"""
    }
    data = client.post_graphql(payload, referer_path=f"/problems/{slug}/")
    if not data:
        return {}
    return (data.get("data", {}) or {}).get("question", {}) or {}

def resolve_meta(qid: int, site_opt: str | None = None) -> dict:
    if requests is None:
        return {}

    sites = [site_opt] if site_opt in ("cn", "com") else ["cn", "com"]
    for s in sites:
        cli = SiteClient(s)
        cli.warm_up()

        slug = lookup_slug_by_id_graphql(qid, cli)
        if not slug:
            slug = lookup_slug_by_id_rest(qid, cli)
        if not slug:
            continue

        q = fetch_question_detail_by_slug(slug, cli)
        if not q:
            continue

        try:
            q_front_id = int(q.get("questionFrontendId") or q.get("questionId") or qid)
        except Exception:
            q_front_id = int(qid)

        title = q.get("title") or ""
        difficulty = q.get("difficulty") or ""
        tags = [t.get("name") for t in (q.get("topicTags") or []) if t.get("name")]

        link = f"{_site_base(s)}/problems/{slug}"

        if s == "cn":
            desc_html = q.get("translatedContent") or q.get("content") or ""
        else:
            desc_html = q.get("content") or q.get("translatedContent") or ""
        desc_text = html_to_text(desc_html)

        skel = {"cpp": None, "python": None}
        for sn in (q.get("codeSnippets") or []):
            lang_slug = (sn.get("langSlug") or "").lower()
            code = sn.get("code") or ""
            if lang_slug == "cpp":
                skel["cpp"] = code
            elif lang_slug in ("python", "python3"):
                skel["python"] = code

        return {
            "id": q_front_id,
            "title": title,
            "slug": slug,
            "difficulty": difficulty,
            "tags": tags,
            "link": link,
            "desc_text": desc_text,
            "skeletons": skel,
            "site": s,
        }
    return {}

# ---------------------- Commands ----------------------

def _comment_block_cpp(meta: dict) -> str:
    desc_lines = (meta.get("desc_text", "") or "").splitlines()
    desc_lines = [" * " + line if line.strip() else " *" for line in desc_lines]
    site_note = f" (source: leetcode.{meta.get('site','com')})"
    return "\n".join([
        "/*",
        f" * LeetCode {meta['id']}. {meta.get('title','')} [{meta.get('difficulty','')}]",
        f" * Link: {meta.get('link','')}{site_note}",
        f" * Tags: {', '.join(meta.get('tags') or [])}",
        " *",
        " * Problem:",
        *desc_lines,
        " *",
        " * Approach: TODO",
        " * Time: O(?), Space: O(?)",
        " */",
        ""
    ])

def _comment_block_py(meta: dict) -> str:
    site_note = f" (source: leetcode.{meta.get('site','com')})"
    desc = meta.get("desc_text", "") or ""
    return f'''"""
LeetCode {meta['id']}. {meta.get('title','')} [{meta.get('difficulty','')}]
Link: {meta.get('link','')}{site_note}
Tags: {", ".join(meta.get('tags') or [])}

Problem:
{desc}

Approach: TODO
Time: O(?), Space: O(?)
"""
'''

def cmd_new(args):
    qid = int(args.id)
    langs = [s.lower() for s in (args.langs or [])]
    site_opt = (args.site or "").lower() or None

    meta = resolve_meta(qid, site_opt)
    if not meta:
        print("[error] failed to fetch problem metadata from both CN and COM.")
        return

    dir_name = sanitize_dir_name(meta["id"], meta.get("slug", ""))
    prob_dir = PROBLEMS / dir_name
    prob_dir.mkdir(parents=True, exist_ok=True)

    write_file(prob_dir / "meta.json", json.dumps(meta, ensure_ascii=False, indent=2))

    for lang in langs:
        if lang in ("cpp", "c++"):
            code = _comment_block_cpp(meta) + (meta.get("skeletons", {}).get("cpp") or "// TODO\n")
            write_file(prob_dir / "cpp" / "v1-solution.cpp", code)
        elif lang in ("py", "python"):
            code = _comment_block_py(meta) + (meta.get("skeletons", {}).get("python") or "# TODO\n")
            write_file(prob_dir / "python" / "v1-solution.py", code)
        elif lang == "java":
            code = _comment_block_cpp(meta) + "class Solution {\n    // TODO\n}\n"
            write_file(prob_dir / "java" / "v1-Solution.java", code)
        else:
            print(f"[warn] unsupported language: {lang}")

    print(f"[ok] created: {prob_dir}")

def cmd_quick(args):
    class A:
        pass
    a = A()
    a.id = args.id
    a.langs = args.langs
    a.site = args.site or None
    cmd_new(a)

def cmd_index(args=None):
    rows = []
    for d in sorted(PROBLEMS.glob("*")):
        if not d.is_dir():
            continue
        meta_path = d / "meta.json"
        if not meta_path.exists():
            try:
                fid = int(d.name.split("-", 1)[0])
            except Exception:
                fid = ""
            rows.append([fid, d.name, "", "", "", str(d.relative_to(ROOT))])
            continue
        try:
            meta = json.loads(meta_path.read_text(encoding="utf-8"))
        except Exception:
            continue
        rows.append([
            meta.get("id", ""),
            meta.get("title", ""),
            meta.get("slug", ""),
            meta.get("difficulty", ""),
            "|".join(meta.get("tags") or []),
            str(d.relative_to(ROOT)),
        ])

    with open(INDEX_CSV, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["id", "title", "slug", "difficulty", "tags", "path"])
        w.writerows(rows)
    print(f"[ok] INDEX.csv updated ({len(rows)} items)")

# ---------- README builders (Overview + Difficulty + Topic) ----------

def _load_all_meta_from_disk():
    metas = []
    for d in sorted(PROBLEMS.glob("*")):
        if not d.is_dir():
            continue
        meta_path = d / "meta.json"
        if not meta_path.exists():
            continue
        try:
            meta = json.loads(meta_path.read_text(encoding="utf-8"))
            meta["_path"] = str(d.relative_to(ROOT))
            metas.append(meta)
        except Exception:
            pass
    return metas

def _markdown_table_header():
    return ["| ID | Title | Difficulty | Tags | Path |",
            "|---:|-------|:----------:|------|------|"]

def _row_line(meta):
    title = meta.get("title", "") or meta.get("slug", "")
    slug = meta.get("slug", "")
    site = meta.get("site", "com")
    site_link = f"{_site_base(site)}/problems/{slug}" if slug else ""
    title_md = f"[{title}]({site_link})" if site_link else title
    tags = ", ".join(meta.get("tags") or [])
    return f"| {meta.get('id','')} | {title_md} | {meta.get('difficulty','')} | {tags} | `{meta.get('_path','')}` |"

def _build_overview_section(metas):
    lines = []
    lines.append("# LeetCode Workspace")
    lines.append("")
    lines.append(f"Last update: {dt.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    lines.append("")
    lines.append("## Overview")
    lines.extend(_markdown_table_header())
    for m in sorted(metas, key=lambda x: int(x.get("id", 0))):
        lines.append(_row_line(m))
    lines.append("")
    return lines

def _build_index_by_difficulty(metas):
    lines = []
    lines.append("## Index by Difficulty")
    order = ["Easy", "Medium", "Hard"]
    buckets = {k: [] for k in order}
    for m in metas:
        d = m.get("difficulty", "")
        if d in buckets:
            buckets[d].append(m)
        else:
            # unknown/None -> add to Medium as neutral bucket
            buckets["Medium"].append(m)
    for diff in order:
        lines.append(f"### {diff} ({len(buckets[diff])})")
        lines.extend(_markdown_table_header())
        for m in sorted(buckets[diff], key=lambda x: int(x.get("id", 0))):
            lines.append(_row_line(m))
        lines.append("")
    return lines

def _build_index_by_topic(metas):
    lines = []
    lines.append("## Index by Topic (Data Structure)")
    topic_map = {}
    for m in metas:
        for t in (m.get("tags") or []):
            topic_map.setdefault(t, []).append(m)
    for topic in sorted(topic_map.keys(), key=lambda s: s.lower()):
        arr = topic_map[topic]
        lines.append(f"### {topic} ({len(arr)})")
        lines.extend(_markdown_table_header())
        for m in sorted(arr, key=lambda x: int(x.get("id", 0))):
            lines.append(_row_line(m))
        lines.append("")
    return lines

def cmd_readme(args=None):
    # Ensure INDEX exists (for CSV users); but categories用的是 meta.json，更丰富
    if not INDEX_CSV.exists():
        cmd_index(None)

    metas = _load_all_meta_from_disk()

    lines = []
    lines.extend(_build_overview_section(metas))
    lines.extend(_build_index_by_difficulty(metas))
    lines.extend(_build_index_by_topic(metas))

    write_file(README_MD, "\n".join(lines))
    print(f"[ok] README.md updated with categorized indices")

def cmd_remove(args):
    target_dir = None
    if args.id is not None:
        prefix = f"{int(args.id):04d}-"
        candidates = [d for d in PROBLEMS.glob(prefix + "*") if d.is_dir()]
        if not candidates:
            print(f"[error] no directory for id {args.id}")
            return
        target_dir = candidates[0]
    elif args.dir:
        target_dir = PROBLEMS / args.dir
        if not target_dir.exists():
            print(f"[error] dir not found: {target_dir}")
            return
    else:
        print("[error] require --id or --dir")
        return

    print(f"[remove] deleting {target_dir}")
    try:
        shutil.rmtree(target_dir)
    except Exception as e:
        print(f"[error] deletion failed: {e}")
        return
    cmd_index(None)
    cmd_readme(None)
    print("[ok] removed")

# ---------------------- CLI ----------------------

def build_cli() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Leet Manager for LeetCode problems (robust CN/COM, CSRF-aware)")
    sub = p.add_subparsers(dest="cmd")

    p_new = sub.add_parser("new", help="create a new problem by id with langs")
    p_new.add_argument("--id", type=int, required=True)
    p_new.add_argument("--site", choices=["com", "cn"], help="force site; default: auto try cn->com")
    p_new.add_argument("--langs", nargs="+", required=True)
    p_new.set_defaults(func=cmd_new)

    p_q = sub.add_parser("quick", help="shortcut: quick <id> <langs...> [--site com|cn]")
    p_q.add_argument("id", type=int)
    p_q.add_argument("langs", nargs="+")
    p_q.add_argument("--site", choices=["com", "cn"], help="force site; default: auto try cn->com")
    p_q.set_defaults(func=cmd_quick)

    p_rm = sub.add_parser("remove", help="remove a problem dir")
    p_rm.add_argument("--id", type=int)
    p_rm.add_argument("--dir")
    p_rm.set_defaults(func=cmd_remove)

    p_idx = sub.add_parser("index", help="rebuild INDEX.csv")
    p_idx.set_defaults(func=cmd_index)

    p_md = sub.add_parser("readme", help="rebuild README.md (with categorized indices)")
    p_md.set_defaults(func=cmd_readme)

    return p

def main(argv=None):
    argv = argv or sys.argv[1:]
    p = build_cli()
    if not argv:
        p.print_help()
        return
    ns = p.parse_args(argv)
    if not hasattr(ns, "func"):
        p.print_help()
        return
    ns.func(ns)

if __name__ == "__main__":
    main()
