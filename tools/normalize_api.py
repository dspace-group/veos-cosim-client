#!/usr/bin/env python3
# Copyright dSPACE SE & Co. KG. All rights reserved.
"""Read clang AST JSON on stdin, emit a stable, sorted public-API snapshot."""

import json
import sys

PREFIX = "DsVeosCoSim"


def qual(node):
    return node.get("type", {}).get("qualType", "")


def walk(node, out):
    kind = node.get("kind")
    name = node.get("name", "")

    if kind == "FunctionDecl" and name.startswith(PREFIX):
        params = [
            qual(c) for c in node.get("inner", []) if c.get("kind") == "ParmVarDecl"
        ]
        ret = qual(node).split("(")[0].strip()
        out.append(f"fn {name}({', '.join(params)}) -> {ret}")
    elif kind in ("RecordDecl", "CXXRecordDecl") and name.startswith(PREFIX):
        fields = [
            f"{c.get('name', '')}: {qual(c)}"
            for c in node.get("inner", [])
            if c.get("kind") == "FieldDecl"
        ]
        out.append(f"struct {name} {{ {'; '.join(fields)} }}")
    elif kind == "EnumDecl" and name.startswith(PREFIX):
        consts = []
        for c in node.get("inner", []):
            if c.get("kind") == "EnumConstantDecl":
                val = ""
                for gc in c.get("inner", []):
                    if "value" in gc:
                        val = gc["value"]
                consts.append(f"{c.get('name', '')}={val}")
        out.append(f"enum {name} {{ {'; '.join(consts)} }}")

    for child in node.get("inner", []) or []:
        walk(child, out)


def main():
    ast = json.load(sys.stdin)
    out = []
    walk(ast, out)
    for line in sorted(set(out)):
        print(line)


if __name__ == "__main__":
    main()
