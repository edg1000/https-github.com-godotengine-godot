def can_build(env, platform):
    return False

def configure(env):
    pass

def get_doc_classes():
    return [
        "BulletPhysicsDirectBodyState",
        "BulletPhysicsServer",
    ]

def get_doc_path():
    return "doc_classes"
