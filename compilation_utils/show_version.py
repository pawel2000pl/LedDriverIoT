#!/usr/bin/python3

import json

with open('resources/version.json') as f:
    print(json.loads(f.read())['version'])
