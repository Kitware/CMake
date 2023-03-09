# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

import os
import re

from dataclasses import dataclass
from typing import Any, cast

# Override much of pygments' CMakeLexer.
# We need to parse CMake syntax definitions, not CMake code.

# For hard test cases that use much of the syntax below, see
# - module/FindPkgConfig.html (with "glib-2.0>=2.10 gtk+-2.0" and similar)
# - module/ExternalProject.html (with http:// https:// git@; also has command options -E --build)
# - manual/cmake-buildsystem.7.html (with nested $<..>; relative and absolute paths, "::")

from pygments.lexers import CMakeLexer
from pygments.token import Name, Operator, Punctuation, String, Text, Comment, Generic, Whitespace, Number
from pygments.lexer import bygroups

# RE to split multiple command signatures
sig_end_re = re.compile(r'(?<=[)])\n')

# Notes on regular expressions below:
# - [\.\+-] are needed for string constants like gtk+-2.0
# - Unix paths are recognized by '/'; support for Windows paths may be added if needed
# - (\\.) allows for \-escapes (used in manual/cmake-language.7)
# - $<..$<..$>..> nested occurrence in cmake-buildsystem
# - Nested variable evaluations are only supported in a limited capacity. Only
#   one level of nesting is supported and at most one nested variable can be present.

CMakeLexer.tokens["root"] = [
  (r'\b(\w+)([ \t]*)(\()', bygroups(Name.Function, Text, Name.Function), '#push'),     # fctn(
  (r'\(', Name.Function, '#push'),
  (r'\)', Name.Function, '#pop'),
  (r'\[', Punctuation, '#push'),
  (r'\]', Punctuation, '#pop'),
  (r'[|;,.=*\-]', Punctuation),
  (r'\\\\', Punctuation),                                   # used in commands/source_group
  (r'[:]', Operator),
  (r'[<>]=', Punctuation),                                  # used in FindPkgConfig.cmake
  (r'\$<', Operator, '#push'),                              # $<...>
  (r'<[^<|]+?>(\w*\.\.\.)?', Name.Variable),                # <expr>
  (r'(\$\w*\{)([^\}\$]*)?(?:(\$\w*\{)([^\}]+?)(\}))?([^\}]*?)(\})',  # ${..} $ENV{..}, possibly nested
    bygroups(Operator, Name.Tag, Operator, Name.Tag, Operator, Name.Tag, Operator)),
  (r'([A-Z]+\{)(.+?)(\})', bygroups(Operator, Name.Tag, Operator)),  # DATA{ ...}
  (r'[a-z]+(@|(://))((\\.)|[\w.+-:/\\])+', Name.Attribute),          # URL, git@, ...
  (r'/\w[\w\.\+-/\\]*', Name.Attribute),                    # absolute path
  (r'/', Name.Attribute),
  (r'\w[\w\.\+-]*/[\w.+-/\\]*', Name.Attribute),            # relative path
  (r'[A-Z]((\\.)|[\w.+-])*[a-z]((\\.)|[\w.+-])*', Name.Builtin), # initial A-Z, contains a-z
  (r'@?[A-Z][A-Z0-9_]*', Name.Constant),
  (r'[a-z_]((\\;)|(\\ )|[\w.+-])*', Name.Builtin),
  (r'[0-9][0-9\.]*', Number),
  (r'(?s)"(\\"|[^"])*"', String),                           # "string"
  (r'\.\.\.', Name.Variable),
  (r'<', Operator, '#push'),                                # <..|..> is different from <expr>
  (r'>', Operator, '#pop'),
  (r'\n', Whitespace),
  (r'[ \t]+', Whitespace),
  (r'#.*\n', Comment),
  #  (r'[^<>\])\}\|$"# \t\n]+', Name.Exception),            # fallback, for debugging only
]

from docutils.utils.code_analyzer import Lexer, LexerError
from docutils.parsers.rst import Directive, directives
from docutils.transforms import Transform
from docutils import io, nodes

from sphinx.directives import ObjectDescription, nl_escape_re
from sphinx.domains import Domain, ObjType
from sphinx.roles import XRefRole
from sphinx.util.nodes import make_refnode
from sphinx.util import logging, ws_re
from sphinx import addnodes

logger = logging.getLogger(__name__)

sphinx_before_1_4 = False
sphinx_before_1_7_2 = False
try:
    from sphinx import version_info
    if version_info < (1, 4):
        sphinx_before_1_4 = True
    if version_info < (1, 7, 2):
        sphinx_before_1_7_2 = True
except ImportError:
    # The `sphinx.version_info` tuple was added in Sphinx v1.2:
    sphinx_before_1_4 = True
    sphinx_before_1_7_2 = True

if sphinx_before_1_7_2:
  # Monkey patch for sphinx generating invalid content for qcollectiongenerator
  # https://github.com/sphinx-doc/sphinx/issues/1435
  from sphinx.util.pycompat import htmlescape
  from sphinx.builders.qthelp import QtHelpBuilder
  old_build_keywords = QtHelpBuilder.build_keywords
  def new_build_keywords(self, title, refs, subitems):
    old_items = old_build_keywords(self, title, refs, subitems)
    new_items = []
    for item in old_items:
      before, rest = item.split("ref=\"", 1)
      ref, after = rest.split("\"")
      if ("<" in ref and ">" in ref):
        new_items.append(before + "ref=\"" + htmlescape(ref) + "\"" + after)
      else:
        new_items.append(item)
    return new_items
  QtHelpBuilder.build_keywords = new_build_keywords

@dataclass
class ObjectEntry:
    docname: str
    objtype: str
    node_id: str
    name: str


class CMakeModule(Directive):
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = True
    option_spec = {'encoding': directives.encoding}

    def __init__(self, *args, **keys):
        self.re_start = re.compile(r'^#\[(?P<eq>=*)\[\.rst:$')
        Directive.__init__(self, *args, **keys)

    def run(self):
        settings = self.state.document.settings
        if not settings.file_insertion_enabled:
            raise self.warning('"%s" directive disabled.' % self.name)

        env = self.state.document.settings.env
        rel_path, path = env.relfn2path(self.arguments[0])
        path = os.path.normpath(path)
        encoding = self.options.get('encoding', settings.input_encoding)
        e_handler = settings.input_encoding_error_handler
        try:
            settings.record_dependencies.add(path)
            f = io.FileInput(source_path=path, encoding=encoding,
                             error_handler=e_handler)
        except UnicodeEncodeError as error:
            msg = ('Problems with "%s" directive path:\n'
                   'Cannot encode input file path "%s" '
                   '(wrong locale?).' % (self.name, path))
            raise self.severe(msg)
        except IOError as error:
            msg = 'Problems with "%s" directive path:\n%s.' % (self.name, error)
            raise self.severe(msg)
        raw_lines = f.read().splitlines()
        f.close()
        rst = None
        lines = []
        for line in raw_lines:
            if rst is not None and rst != '#':
                # Bracket mode: check for end bracket
                pos = line.find(rst)
                if pos >= 0:
                    if line[0] == '#':
                        line = ''
                    else:
                        line = line[0:pos]
                    rst = None
            else:
                # Line mode: check for .rst start (bracket or line)
                m = self.re_start.match(line)
                if m:
                    rst = ']%s]' % m.group('eq')
                    line = ''
                elif line == '#.rst:':
                    rst = '#'
                    line = ''
                elif rst == '#':
                    if line == '#' or line[:2] == '# ':
                        line = line[2:]
                    else:
                        rst = None
                        line = ''
                elif rst is None:
                    line = ''
            lines.append(line)
        if rst is not None and rst != '#':
            raise self.warning('"%s" found unclosed bracket "#[%s[.rst:" in %s' %
                               (self.name, rst[1:-1], path))
        self.state_machine.insert_input(lines, path)
        return []

class _cmake_index_entry:
    def __init__(self, desc):
        self.desc = desc

    def __call__(self, title, targetid, main = 'main'):
        # See https://github.com/sphinx-doc/sphinx/issues/2673
        if sphinx_before_1_4:
            return ('pair', u'%s ; %s' % (self.desc, title), targetid, main)
        else:
            return ('pair', u'%s ; %s' % (self.desc, title), targetid, main, None)

_cmake_index_objs = {
    'command':    _cmake_index_entry('command'),
    'cpack_gen':  _cmake_index_entry('cpack generator'),
    'envvar':     _cmake_index_entry('envvar'),
    'generator':  _cmake_index_entry('generator'),
    'genex':      _cmake_index_entry('genex'),
    'guide':      _cmake_index_entry('guide'),
    'manual':     _cmake_index_entry('manual'),
    'module':     _cmake_index_entry('module'),
    'policy':     _cmake_index_entry('policy'),
    'prop_cache': _cmake_index_entry('cache property'),
    'prop_dir':   _cmake_index_entry('directory property'),
    'prop_gbl':   _cmake_index_entry('global property'),
    'prop_inst':  _cmake_index_entry('installed file property'),
    'prop_sf':    _cmake_index_entry('source file property'),
    'prop_test':  _cmake_index_entry('test property'),
    'prop_tgt':   _cmake_index_entry('target property'),
    'variable':   _cmake_index_entry('variable'),
    }

class CMakeTransform(Transform):

    # Run this transform early since we insert nodes we want
    # treated as if they were written in the documents.
    default_priority = 210

    def __init__(self, document, startnode):
        Transform.__init__(self, document, startnode)
        self.titles = {}

    def parse_title(self, docname):
        """Parse a document title as the first line starting in [A-Za-z0-9<$]
           or fall back to the document basename if no such line exists.
           The cmake --help-*-list commands also depend on this convention.
           Return the title or False if the document file does not exist.
        """
        env = self.document.settings.env
        title = self.titles.get(docname)
        if title is None:
            fname = os.path.join(env.srcdir, docname+'.rst')
            try:
                f = open(fname, 'r')
            except IOError:
                title = False
            else:
                for line in f:
                    if len(line) > 0 and (line[0].isalnum() or line[0] == '<' or line[0] == '$'):
                        title = line.rstrip()
                        break
                f.close()
                if title is None:
                    title = os.path.basename(docname)
            self.titles[docname] = title
        return title

    def apply(self):
        env = self.document.settings.env

        # Treat some documents as cmake domain objects.
        objtype, sep, tail = env.docname.partition('/')
        make_index_entry = _cmake_index_objs.get(objtype)
        if make_index_entry:
            title = self.parse_title(env.docname)
            # Insert the object link target.
            if objtype == 'command':
                targetname = title.lower()
            elif objtype == 'guide' and not tail.endswith('/index'):
                targetname = tail
            else:
                if objtype == 'genex':
                    m = CMakeXRefRole._re_genex.match(title)
                    if m:
                        title = m.group(1)
                targetname = title
            targetid = '%s:%s' % (objtype, targetname)
            targetnode = nodes.target('', '', ids=[targetid])
            self.document.note_explicit_target(targetnode)
            self.document.insert(0, targetnode)
            # Insert the object index entry.
            indexnode = addnodes.index()
            indexnode['entries'] = [make_index_entry(title, targetid)]
            self.document.insert(0, indexnode)

            # Add to cmake domain object inventory
            domain = cast(CMakeDomain, env.get_domain('cmake'))
            domain.note_object(objtype, targetname, targetid, targetid)

class CMakeObject(ObjectDescription):

    def handle_signature(self, sig, signode):
        # called from sphinx.directives.ObjectDescription.run()
        signode += addnodes.desc_name(sig, sig)
        if self.objtype == 'genex':
            m = CMakeXRefRole._re_genex.match(sig)
            if m:
                sig = m.group(1)
        return sig

    def add_target_and_index(self, name, sig, signode):
        if self.objtype == 'command':
            targetname = name.lower()
        else:
            targetname = name
        targetid = '%s:%s' % (self.objtype, targetname)
        if targetid not in self.state.document.ids:
            signode['names'].append(targetid)
            signode['ids'].append(targetid)
            signode['first'] = (not self.names)
            self.state.document.note_explicit_target(signode)

            domain = cast(CMakeDomain, self.env.get_domain('cmake'))
            domain.note_object(self.objtype, targetname, targetid, targetid,
                               location=signode)

        make_index_entry = _cmake_index_objs.get(self.objtype)
        if make_index_entry:
            self.indexnode['entries'].append(make_index_entry(name, targetid))

class CMakeSignatureObject(CMakeObject):
    object_type = 'signature'

    option_spec = {
        'target': directives.unchanged,
    }

    def get_signatures(self):
        content = nl_escape_re.sub('', self.arguments[0])
        lines = sig_end_re.split(content)
        return [ws_re.sub(' ', line.strip()) for line in lines]

    def handle_signature(self, sig, signode):
        language = 'cmake'
        classes = ['code', 'cmake', 'highlight']

        node = addnodes.desc_name(sig, '', classes=classes)

        try:
            tokens = Lexer(sig, language, 'short')
        except LexerError as error:
            if self.state.document.settings.report_level > 2:
                # Silently insert without syntax highlighting.
                tokens = Lexer(sig, language, 'none')
            else:
                raise self.warning(error)

        for classes, value in tokens:
            if classes:
                node += nodes.inline(value, value, classes=classes)
            else:
                node += nodes.Text(value)

        signode.clear()
        signode += node

        return sig

    def __init__(self, *args, **kwargs):
        self.targetnames = {}
        super().__init__(*args, **kwargs)

    def add_target_and_index(self, name, sig, signode):
        if name in self.targetnames:
            sigargs = self.targetnames[name]
        else:
            def extract_keywords(params):
                for p in params:
                    if p[0].isalpha():
                        yield p
                    else:
                        return

            keywords = extract_keywords(name.split('(')[1].split())
            sigargs = ' '.join(keywords)
        targetname = sigargs.lower()
        targetid = nodes.make_id(targetname)

        if targetid not in self.state.document.ids:
            signode['names'].append(targetname)
            signode['ids'].append(targetid)
            signode['first'] = (not self.names)
            self.state.document.note_explicit_target(signode)

            # Register the signature as a command object.
            command = name.split('(')[0].lower()
            refname = f'{command}({sigargs})'
            refid = f'command:{command}({targetname})'

            domain = cast(CMakeDomain, self.env.get_domain('cmake'))
            domain.note_object('command', name=refname, target_id=refid,
                               node_id=targetid, location=signode)

    def run(self):
        targets = self.options.get('target')
        if targets is not None:
            signatures = self.get_signatures()
            targets = [t.strip() for t in targets.split('\n')]
            for signature, target in zip(signatures, targets):
                self.targetnames[signature] = target

        return super().run()

class CMakeXRefRole(XRefRole):

    # See sphinx.util.nodes.explicit_title_re; \x00 escapes '<'.
    _re = re.compile(r'^(.+?)(\s*)(?<!\x00)<(.*?)>$', re.DOTALL)
    _re_ref = re.compile(r'^.*\s<\w+([(][\w\s]+[)])?>$', re.DOTALL)
    _re_genex = re.compile(r'^\$<([^<>:]+)(:[^<>]+)?>$', re.DOTALL)
    _re_guide = re.compile(r'^([^<>/]+)/([^<>]*)$', re.DOTALL)

    def __call__(self, typ, rawtext, text, *args, **keys):
        if typ == 'cmake:command':
            m = CMakeXRefRole._re_ref.match(text)
            if m is None:
                text = f'{text} <{text}>'
        elif typ == 'cmake:genex':
            m = CMakeXRefRole._re_genex.match(text)
            if m:
                text = '%s <%s>' % (text, m.group(1))
        elif typ == 'cmake:guide':
            m = CMakeXRefRole._re_guide.match(text)
            if m:
                text = '%s <%s>' % (m.group(2), text)
        # CMake cross-reference targets frequently contain '<' so escape
        # any explicit `<target>` with '<' not preceded by whitespace.
        while True:
            m = CMakeXRefRole._re.match(text)
            if m and len(m.group(2)) == 0:
                text = '%s\x00<%s>' % (m.group(1), m.group(3))
            else:
                break
        return XRefRole.__call__(self, typ, rawtext, text, *args, **keys)

    # We cannot insert index nodes using the result_nodes method
    # because CMakeXRefRole is processed before substitution_reference
    # nodes are evaluated so target nodes (with 'ids' fields) would be
    # duplicated in each evaluated substitution replacement.  The
    # docutils substitution transform does not allow this.  Instead we
    # use our own CMakeXRefTransform below to add index entries after
    # substitutions are completed.
    #
    # def result_nodes(self, document, env, node, is_ref):
    #     pass

class CMakeXRefTransform(Transform):

    # Run this transform early since we insert nodes we want
    # treated as if they were written in the documents, but
    # after the sphinx (210) and docutils (220) substitutions.
    default_priority = 221

    def apply(self):
        env = self.document.settings.env

        # Find CMake cross-reference nodes and add index and target
        # nodes for them.
        for ref in self.document.traverse(addnodes.pending_xref):
            if not ref['refdomain'] == 'cmake':
                continue

            objtype = ref['reftype']
            make_index_entry = _cmake_index_objs.get(objtype)
            if not make_index_entry:
                continue

            objname = ref['reftarget']
            if objtype == 'guide' and CMakeXRefRole._re_guide.match(objname):
                # Do not index cross-references to guide sections.
                continue

            if objtype == 'command':
                # Index signature references to their parent command.
                objname = objname.split('(')[0].lower()

            targetnum = env.new_serialno('index-%s:%s' % (objtype, objname))

            targetid = 'index-%s-%s:%s' % (targetnum, objtype, objname)
            targetnode = nodes.target('', '', ids=[targetid])
            self.document.note_explicit_target(targetnode)

            indexnode = addnodes.index()
            indexnode['entries'] = [make_index_entry(objname, targetid, '')]
            ref.replace_self([indexnode, targetnode, ref])

class CMakeDomain(Domain):
    """CMake domain."""
    name = 'cmake'
    label = 'CMake'
    object_types = {
        'command':    ObjType('command',    'command'),
        'cpack_gen':  ObjType('cpack_gen',  'cpack_gen'),
        'envvar':     ObjType('envvar',     'envvar'),
        'generator':  ObjType('generator',  'generator'),
        'genex':      ObjType('genex',      'genex'),
        'guide':      ObjType('guide',      'guide'),
        'variable':   ObjType('variable',   'variable'),
        'module':     ObjType('module',     'module'),
        'policy':     ObjType('policy',     'policy'),
        'prop_cache': ObjType('prop_cache', 'prop_cache'),
        'prop_dir':   ObjType('prop_dir',   'prop_dir'),
        'prop_gbl':   ObjType('prop_gbl',   'prop_gbl'),
        'prop_inst':  ObjType('prop_inst',  'prop_inst'),
        'prop_sf':    ObjType('prop_sf',    'prop_sf'),
        'prop_test':  ObjType('prop_test',  'prop_test'),
        'prop_tgt':   ObjType('prop_tgt',   'prop_tgt'),
        'manual':     ObjType('manual',     'manual'),
    }
    directives = {
        'command':    CMakeObject,
        'envvar':     CMakeObject,
        'genex':      CMakeObject,
        'signature':  CMakeSignatureObject,
        'variable':   CMakeObject,
        # Other `object_types` cannot be created except by the `CMakeTransform`
    }
    roles = {
        'command':    CMakeXRefRole(fix_parens = True, lowercase = True),
        'cpack_gen':  CMakeXRefRole(),
        'envvar':     CMakeXRefRole(),
        'generator':  CMakeXRefRole(),
        'genex':      CMakeXRefRole(),
        'guide':      CMakeXRefRole(),
        'variable':   CMakeXRefRole(),
        'module':     CMakeXRefRole(),
        'policy':     CMakeXRefRole(),
        'prop_cache': CMakeXRefRole(),
        'prop_dir':   CMakeXRefRole(),
        'prop_gbl':   CMakeXRefRole(),
        'prop_inst':  CMakeXRefRole(),
        'prop_sf':    CMakeXRefRole(),
        'prop_test':  CMakeXRefRole(),
        'prop_tgt':   CMakeXRefRole(),
        'manual':     CMakeXRefRole(),
    }
    initial_data = {
        'objects': {},  # fullname -> docname, objtype
    }

    def clear_doc(self, docname):
        to_clear = set()
        for fullname, obj in self.data['objects'].items():
            if obj.docname == docname:
                to_clear.add(fullname)
        for fullname in to_clear:
            del self.data['objects'][fullname]

    def resolve_xref(self, env, fromdocname, builder,
                     typ, target, node, contnode):
        targetid = f'{typ}:{target}'
        obj = self.data['objects'].get(targetid)

        if obj is None and typ == 'command':
            # If 'command(args)' wasn't found, try just 'command'.
            # TODO: remove this fallback? warn?
            # logger.warning(f'no match for {targetid}')
            command = target.split('(')[0]
            targetid = f'{typ}:{command}'
            obj = self.data['objects'].get(targetid)

        if obj is None:
            # TODO: warn somehow?
            return None

        return make_refnode(builder, fromdocname, obj.docname, obj.node_id,
                            contnode, target)

    def note_object(self, objtype: str, name: str, target_id: str,
                    node_id: str, location: Any = None):
        if target_id in self.data['objects']:
            other = self.data['objects'][target_id].docname
            logger.warning(
                f'CMake object {target_id!r} also described in {other!r}',
                location=location)

        self.data['objects'][target_id] = ObjectEntry(
            self.env.docname, objtype, node_id, name)

    def get_objects(self):
        for refname, obj in self.data['objects'].items():
            yield (refname, obj.name, obj.objtype, obj.docname, obj.node_id, 1)

def setup(app):
    app.add_directive('cmake-module', CMakeModule)
    app.add_transform(CMakeTransform)
    app.add_transform(CMakeXRefTransform)
    app.add_domain(CMakeDomain)
    return {"parallel_read_safe": True}
