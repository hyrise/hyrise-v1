docker kill hyriseDev
docker rm hyriseDev
docker run -d -p 2222:22 -p 5000:5000 -p 8888:8888 --name hyriseDev -h hyriseDev -v ~/workspace:/home/dev/workspace -v ~/.ssh:/home/dev/.ssh hyrise/hyrisedev:latest
echo "waiting for container to start..."
sleep 3
ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no dev@localhost -p2222