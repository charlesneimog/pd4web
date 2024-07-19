import os

import pygit2

# TODO: Replace git with pygit2


class Git:
    def __init__(self, repo_url, local_path):
        self.repo_url = repo_url
        self.local_path = local_path
        self.CloneRepositoryRecursively()

    def CloneRepositoryRecursively(self):
        # Clonando o repositório principal
        repo = pygit2.clone_repository(self.repo_url, self.local_path)


# remove old repo if it exists
if os.path.exists("/home/neimog/Documents/pd4web-t"):
    os.system("rm -rf /home/neimog/Documents/pd4web-t")

Pd4WebGit = Git(
    "https://github.com/charlesneimog/py4pd", "/home/neimog/Documents/pd4web-t"
)
