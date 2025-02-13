import yaml
import requests
import os

GITHUB_API_URL = "https://api.github.com"
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
if not GITHUB_TOKEN:
    raise Exception("GITHUB_TOKEN não encontrado nas variáveis de ambiente.")

def get_yaml_content(file_path):
    with open(file_path, 'r') as file:
        return yaml.safe_load(file)

def create_github_issue(repo, title, body):
    url = f"{GITHUB_API_URL}/repos/{repo}/issues"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }
    print(body)
    payload = {
        "title": title,
        "body": body
    }
    response = requests.post(url, headers=headers, json=payload)
    if response.status_code == 201:
        print(f"Issue created successfully: {response.json()['html_url']}")
    else:
        print(f"Failed to create issue: {response.content}")

def check_for_updates(libraries):
    issues = []
    for lib in libraries:
        developer = lib['Developer']
        repo = lib['Repository']
        version = lib['Version']

        if version.startswith('v'): # It's a tag
            url = f"{GITHUB_API_URL}/repos/{developer}/{repo}/tags"
        else: # It's a commit
            url = f"{GITHUB_API_URL}/repos/{developer}/{repo}/commits/{version}"

        headers = {
            "Authorization": f"token {GITHUB_TOKEN}",
            "Accept": "application/vnd.github.v3+json"
        }
        response = requests.get(url, headers=headers)
        if response.status_code == 200:
            data = response.json()
            if version.startswith('v'):
                latest_tag = data[0]['name']
                if latest_tag != version:
                    title = f"New tag detected in {repo}"
                    body = f"A new tag {latest_tag} has been detected in {repo}. The previous version was {version}."
                    issues.append((title, body))
            else:
                latest_commit = data['sha']
                if latest_commit != version:
                    title = f"New commit detected in {repo}"
                    body = f"A new commit {latest_commit} has been detected in {repo}. The previous version was {version}."
                    issues.append((title, body))
        else:
            print(f"Failed to get data for {repo}: {response.content}")

    text = ""
    for title, body in issues:
        text += f"### {title}\n{body}\n\n\n"

    create_github_issue(f"charlesneimog/pd4web", "New Updates for Supported Libraries", text)


if __name__ == "__main__":
    yaml_content = get_yaml_content('Sources/Libraries/Libraries.yaml')
    libraries = yaml_content['Libraries']
    check_for_updates(libraries)
