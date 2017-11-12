"""scons.Node.Python

Python nodes.

"""

#
# Copyright (c) 2001 - 2017 The SCons Foundation
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

__revision__ = "src/engine/SCons/Node/Python.py rel_3.0.0:4395:8972f6a2f699 2017/09/18 12:59:24 bdbaddog"

import SCons.Node

class ValueNodeInfo(SCons.Node.NodeInfoBase):
    __slots__ = ('csig',)
    current_version_id = 2

    field_list = ['csig']

    def str_to_node(self, s):
        return Value(s)

    def __getstate__(self):
        """
        Return all fields that shall be pickled. Walk the slots in the class
        hierarchy and add those to the state dictionary. If a '__dict__' slot is
        available, copy all entries to the dictionary. Also include the version
        id, which is fixed for all instances of a class.
        """
        state = getattr(self, '__dict__', {}).copy()
        for obj in type(self).mro():
            for name in getattr(obj,'__slots__',()):
                if hasattr(self, name):
                    state[name] = getattr(self, name)

        state['_version_id'] = self.current_version_id
        try:
            del state['__weakref__']
        except KeyError:
            pass

        return state

    def __setstate__(self, state):
        """
        Restore the attributes from a pickled state.
        """
        # TODO check or discard version
        del state['_version_id']
        for key, value in state.items():
            if key not in ('__weakref__',):
                setattr(self, key, value)


class ValueBuildInfo(SCons.Node.BuildInfoBase):
    __slots__ = ()
    current_version_id = 2

class Value(SCons.Node.Node):
    """A class for Python variables, typically passed on the command line
    or generated by a script, but not from a file or some other source.
    """

    NodeInfo = ValueNodeInfo
    BuildInfo = ValueBuildInfo

    def __init__(self, value, built_value=None):
        SCons.Node.Node.__init__(self)
        self.value = value
        self.changed_since_last_build = 6
        self.store_info = 0
        if built_value is not None:
            self.built_value = built_value

    def str_for_display(self):
        return repr(self.value)

    def __str__(self):
        return str(self.value)

    def make_ready(self):
        self.get_csig()

    def build(self, **kw):
        if not hasattr(self, 'built_value'):
            SCons.Node.Node.build(self, **kw)

    is_up_to_date = SCons.Node.Node.children_are_up_to_date

    def is_under(self, dir):
        # Make Value nodes get built regardless of
        # what directory scons was run from. Value nodes
        # are outside the filesystem:
        return 1

    def write(self, built_value):
        """Set the value of the node."""
        self.built_value = built_value

    def read(self):
        """Return the value. If necessary, the value is built."""
        self.build()
        if not hasattr(self, 'built_value'):
            self.built_value = self.value
        return self.built_value

    def get_text_contents(self):
        """By the assumption that the node.built_value is a
        deterministic product of the sources, the contents of a Value
        are the concatenation of all the contents of its sources.  As
        the value need not be built when get_contents() is called, we
        cannot use the actual node.built_value."""
        ###TODO: something reasonable about universal newlines
        contents = str(self.value)
        for kid in self.children(None):
            contents = contents + kid.get_contents().decode()
        return contents

    def get_contents(self):
        text_contents = self.get_text_contents()
        try:
            return text_contents.encode()
        except UnicodeDecodeError:
            # Already encoded as python2 str are bytes
            return text_contents


    def changed_since_last_build(self, target, prev_ni):
        cur_csig = self.get_csig()
        try:
            return cur_csig != prev_ni.csig
        except AttributeError:
            return 1

    def get_csig(self, calc=None):
        """Because we're a Python value node and don't have a real
        timestamp, we get to ignore the calculator and just use the
        value contents."""
        try:
            return self.ninfo.csig
        except AttributeError:
            pass
        contents = self.get_contents()
        self.get_ninfo().csig = contents
        return contents

# Local Variables:
# tab-width:4
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=4 shiftwidth=4:
