def can_build(env, platform):
    return True


def configure(env):
    pass


def get_doc_classes():
<<<<<<< HEAD
    return [
        "WebRTCPeerConnection",
        "WebRTCDataChannel",
        "WebRTCMultiplayer",
    ]
=======
    return ["WebRTCPeerConnection", "WebRTCDataChannel", "WebRTCMultiplayer"]

>>>>>>> audio-bus-effect-fixed


def get_doc_path():
    return "doc_classes"
