def get_name() -> str:
    return 'gridmap'
    
def can_build(env: dict) -> bool:
    return True


def get_doc_classes() -> [str]:
    return [
        "GridMap",
    ]


def get_doc_path() -> str:
    return "doc_classes"
