# Fail fast
set -o errexit
set -o pipefail
set -o nounset

readonly ssh_dir="/root/.ssh"
mkdir -p "${ssh_dir}"

echo "Adding known hosts"
if [[ -z "${INPUT_SSH_KNOWN_HOSTS}" ]]; then
    ssh-keyscan -t rsa github.com > "${ssh_dir}/known_hosts"
else
    echo "${INPUT_SSH_KNOWN_HOSTS}" > "${ssh_dir}/known_hosts"
fi

echo "Adding SSH key"
ssh-agent -a "${SSH_AUTH_SOCK}" > /dev/null
echo "${INPUT_SSH_KEY}" | ssh-add -

echo "Setting up git credentials"
git config --global user.name "${INPUT_GIT_USER}"
git config --global user.email "${INPUT_GIT_EMAIL}"

echo "Cloning deploy repository"
readonly repository_dir="$(mktemp -d)"
readonly deploy_dir="${repository_dir}/${INPUT_REPOSITORY_SUBDIR}"
git clone --depth 10 "${INPUT_REPOSITORY}" "${repository_dir}"
if [ -z "${INPUT_REPOSITORY_BRANCH-}" ]; then
    git -C "${repository_dir}" checkout "${INPUT_REPOSITORY_BRANCH}"
fi

if [ "${INPUT_DELETE_EXISTING:-false}" = "true" ]; then
    echo "Removing existing files"
    rm -rf "${deploy_dir}"
fi

echo "Copying files"
mkdir -p "${deploy_dir}"
cp -R "${INPUT_LOCAL_DIR}" "${deploy_dir}"

cd "${repository_dir}"
git add --all
if git diff --quiet --cached; then
    echo "No changes to commit"
else
    echo "Creating and pushing commit"
    # Evaluate to resolve inner variables
    readonly commit_msg="$(eval echo -n ${INPUT_COMMIT_MESSAGE})"
    git commit -m "${commit_msg}"
    git push origin
fi
