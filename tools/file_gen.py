import os
import json
import argparse

CONFIG_FILE = "file_gen_config.json"

class Colors:
    WARNING = '\033[93m'  # жёлтый
    SUCCESS = '\033[92m'  # зелёный
    RESET = '\033[0m'     # сброс


def load_config(path=CONFIG_FILE):
    if os.path.exists(path):
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    return {}


def file_exists_warning(path: str) -> bool:
    if os.path.exists(path):
        print(f"{Colors.WARNING}[WARNING]{Colors.RESET} File already exists and will NOT be overwritten: {path}")
        return True
    return False


def find_folder_prefix(directory: str, folder_prefixes: dict, fallback: str) -> str:
    parts = directory.replace("\\", "/").split("/")
    for p in parts:
        if p in folder_prefixes:
            return folder_prefixes[p]
    return fallback


def detect_src_segment(directory: str, folder_prefixes: dict, project_root: str):
    rel_path = os.path.relpath(directory, project_root).replace("\\", "/")
    parts = rel_path.split("/")

    root_idx = None
    for i, p in enumerate(parts):
        if p in folder_prefixes:
            root_idx = i
            break

    if root_idx is None:
        return False

    if len(parts) > root_idx + 1 and parts[root_idx + 1].lower() == "src":
        return True
    else:
        print(f"{Colors.WARNING}[WARNING]{Colors.RESET} 'src' is expected immediately after root folder (e.g., Engine/src/...)")
        return False


def build_guard(prefix: str, directory: str, filename: str, project_root: str, folder_prefixes: dict) -> str:
    abs_path = os.path.join(project_root, directory, filename)
    rel_path = os.path.relpath(abs_path, project_root).replace("\\", "/")
    parts = rel_path.split("/")

    try:
        src_idx = parts.index("src")
        dir_after_src = parts[src_idx + 1:-1]  
        middle = "_".join(p.upper() for p in dir_after_src)
        guard = f"{prefix}_SRC_{middle}_{filename.upper()}_HPP" if middle else f"{prefix}_SRC_{filename.upper()}_HPP"
    except ValueError:
        dir_parts = parts[:-1]
        middle = "_".join(p.upper() for p in dir_parts if p not in folder_prefixes)
        guard = f"{prefix}_{middle}_{filename.upper()}_HPP" if middle else f"{prefix}_{filename.upper()}_HPP"

    return guard


def get_namespace_from_config(directory: str, namespaces: dict, project_root: str) -> str:
    rel_path = os.path.relpath(directory, project_root).replace("\\", "/")
    parts = rel_path.split("/")

    for segment in parts:
        if segment in namespaces:
            return namespaces[segment]
    return ""


def create_files(directory: str, filename: str, fallback_prefix: str, project_root: str, folder_prefixes: dict, namespaces: dict):
    abs_directory = os.path.join(project_root, directory)
    os.makedirs(abs_directory, exist_ok=True)


    prefix = find_folder_prefix(directory, folder_prefixes, fallback_prefix)
    guard = build_guard(prefix, directory, filename, project_root, folder_prefixes)

    
    hpp_path = os.path.join(abs_directory, f"{filename}.hpp")
    cpp_path = os.path.join(abs_directory, f"{filename}.cpp")

    hpp_exists = file_exists_warning(hpp_path)
    cpp_exists = file_exists_warning(cpp_path)

    class_name = filename
    namespace = get_namespace_from_config(directory, namespaces, project_root)

    indent = "    "  

    if not hpp_exists:
        with open(hpp_path, "w", encoding="utf-8") as f:
            f.write(f"#ifndef {guard}\n")
            f.write(f"#define {guard}\n\n")

            if namespace:
                f.write(f"namespace {namespace} \n{{\n")

            f.write(f"{indent}class {class_name}\n")
            f.write(f"{indent}" + "{\n")
            f.write(f"{indent}public:\n")
            f.write(f"{indent*2}{class_name}();\n")
            f.write(f"{indent*2}~{class_name}();\n")
            f.write(f"{indent}" + "};\n")

            if namespace:
                f.write(f"}}; // namespace {namespace}\n\n")  

            f.write(f"#endif // {guard}\n")

    if not cpp_exists:
        with open(cpp_path, "w", encoding="utf-8") as f:
            f.write(f'#include "{filename}.hpp"\n\n')

            if namespace:
                f.write(f"namespace {namespace} \n{{\n")
                indent_cpp = "    "
            else:
                indent_cpp = ""

            f.write(f"{indent_cpp}{class_name}::{class_name}() {{}}\n")
            f.write(f"{indent_cpp}{class_name}::~{class_name}() {{}}\n")

            if namespace:
                f.write(f"}}; // namespace {namespace}\n")

    if not hpp_exists or not cpp_exists:
        print(f"{Colors.SUCCESS}[SUCCESS]{Colors.RESET} Files created:\n  {hpp_path}\n  {cpp_path}")
    else:
        print(f"{Colors.WARNING}[WARNING]{Colors.RESET} No files were created, they already exist.")


if __name__ == "__main__":
    config = load_config()

    parser = argparse.ArgumentParser(description="C++ file generator")
    parser.add_argument("directory")
    parser.add_argument("filename")
    parser.add_argument("--guard_prefix", default=config.get("guard_prefix", "NB"))
    parser.add_argument("--project_root", default=None)  

    args = parser.parse_args()

    project_root = args.project_root if args.project_root else config.get("project_root", os.getcwd())
    print(f"Project root: {project_root}")
    folder_prefixes = config.get("folder_prefixes", {})
    namespaces = config.get("namespaces", {})

    create_files(args.directory, args.filename, args.guard_prefix, project_root, folder_prefixes, namespaces)
