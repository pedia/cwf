Python interface to the ctemplate library
=========================================

The main functions and classes of the ctemplate library are
wrapped as native Python objects.
This enables quick prototyping and testing of a ctemplate
system.

Overview
========
Run python internal help() for an API overview:
$ python -c "import ctemplate; help(ctemplate)"

Example:

# loads example.tpl in current directory
template = ctemplate.Template("example.tpl", ctemplate.DO_NOT_STRIP)
dictionary = ctemplate.Dictionary("my example dict")
dictionary.SetValue("VALUE1", "TEST1")
# dict setters call SetValue() automatically
dictionary["VALUE2"] = "TEST2"
# all objects except booleans are converted to strings
dictionary["NUMBER"] = 87411
dictionary["TUPLE"] = (1, 2, 3)
# Sections
dictionary.ShowSection("A_SECTION")
# boolean True calls ShowSection() automatically
dictionary["IN_CA"] = True
# boolean False is ignored (ie. this statement has no effect)
dictionary["IGNORED"] = False
# And of course the expand function
print template.Expand(dictionary)

Installation
============
Run "python setup.py install". See "python setup.py install --help" for
options.

Memory
======
The cached pages are only deleted when the Python interpreter exits,
via a google::Template::ClearCache() call. So you don't want to use
python-ctemplate on long-running python processes with an ever
growing template list.

