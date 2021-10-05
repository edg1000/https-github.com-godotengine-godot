def can_build(env, platform):
    return env.module_check_dependencies("vorbis", ["ogg"])


def configure(env):
    pass


def get_doc_classes():
    return [
        "AudioStreamOGGVorbis",
        "AudioStreamPlaybackOGGVorbis",
    ]


def get_doc_path():
    return "doc_classes"


def get_module_dependencies():
    return [
        "ogg",
    ]
