#!/usr/bin/python
import os
import sys
sys.path.insert(0, os.getcwd())
import ctemplate

def test_constants ():
    print "DO_NOT_STRIP", ctemplate.DO_NOT_STRIP
    print "STRIP_BLANK_LINES", ctemplate.STRIP_BLANK_LINES
    print "STRIP_WHITESPACE", ctemplate.STRIP_WHITESPACE
    print "TS_EMPTY", ctemplate.TS_EMPTY
    print "TS_ERROR", ctemplate.TS_ERROR
    print "TS_READY", ctemplate.TS_READY
    print "TS_SHOULD_RELOAD", ctemplate.TS_SHOULD_RELOAD

def test_global ():
    ctemplate.SetGlobalValue("GLOBAL_FOO", "bar")
    ctemplate.SetGlobalValue("GLOBAL_INT", 1)
    ctemplate.SetGlobalValue("GLOBAL_LONG", 2L)

def test_methods (dictionary):
    dictionary.SetValue("METHOD_FOO", "baz")
    dictionary.SetValue("METHOD_INT", 4)
    dictionary.SetValue("METHOD_LONG", 5L)

def test_escape (dictionary):
    dictionary.SetValue("ESCAPE_HTML", "<baz>")
    dictionary.SetValue("ESCAPE_XML", "&nbsp;")
    dictionary.SetValue("ESCAPE_JS", "'baz'")
    dictionary.SetValue("ESCAPE_JSON", "'baz'")

def test_dict (dictionary):
    dictionary["DICT_FOO"] = "87411"
    dictionary["DICT_INT"] = 7411
    dictionary["DICT_LONG"] = 7412L
    dictionary["DICT_TUPLE"] = (1, 2, 3)

def test_section (dictionary):
    dictionary.ShowSection("SECT1")
    dictionary["SECT2"] = True
    dictionary["SECT3"] = False

def test_subdict (dictionary):
    sub_dict = dictionary.AddSectionDictionary("SUB1")
    sub_dict["SUB_FOO"] = "bar1"
    sub_dict = dictionary.AddSectionDictionary("SUB1")
    sub_dict["SUB_FOO"] = "bar2"

def main ():
    test_constants()
    filename = os.path.join("tests", "test.tpl")
    ctemplate.RegisterTemplate(filename)
    template = ctemplate.Template(filename, ctemplate.DO_NOT_STRIP)
    test_global()
    dictionary = ctemplate.Dictionary("my example dict")
    dictionary.SetFilename(filename)
    test_methods(dictionary)
    test_escape(dictionary)
    test_dict(dictionary)
    test_section(dictionary)
    test_subdict(dictionary)
    print dictionary.Dump()
    print template.Expand(dictionary)
    print "Errors", ctemplate.GetBadSyntaxList(True, ctemplate.DO_NOT_STRIP)


if __name__ == '__main__':
    main()
