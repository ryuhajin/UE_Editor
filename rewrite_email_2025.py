# rewrite_email_2025.py
from datetime import datetime

def commit_callback(commit):
    # 커밋의 committer_date(타임스탬프)를 날짜로 변환
    date = datetime.utcfromtimestamp(commit.committer_date)
    # 2025년 커밋만 이메일 변경
    if 2025 <= date.year < 2026:
        commit.author_email = b"eyesibar21@gmail.com"
        commit.committer_email = b"eyesibar21@gmail.com"
