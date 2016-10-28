import click
import requests
from requests.compat import urljoin

def add_node(url, nodeId):
    url = urljoin(url, "add_node")
    if nodeId % 10000 == 0:
        print("add_node", url, nodeId)
    return requests.post(url, json={'node_id': nodeId})

def checkpoint(url):
    return requests.post(url, urljoin(url, "checkpoint"))

@click.command()
@click.argument("url")
def fill_log(url):
    print("URL: ", url)

    nodeId = 0
    while True:
        response = add_node(url, nodeId)
        if response.status_code == 507:
            break

        assert response.status_code == requests.codes.ok, response.status_code

        nodeId += 1

    print("Server log fill up to", nodeId - 1)

if __name__ == '__main__':
    fill_log()
