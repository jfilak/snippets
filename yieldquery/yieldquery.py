#!/usr/bin/python

class YieldQuery(object):

    def __init__(self, data, size):
        self._data = data
        self._size = size

    def __len__(self):
        return self._size

    def __iter__(self):
        return iter(self._data)


if __name__ == "__main__":
    yq = YieldQuery(["a", "b", "c"], 3)

    print(len(yq))
    for c in yq:
        print(c)
