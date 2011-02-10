/*
Copyright (c) 2007 Bastian Kleineidam
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of Bastian Kleineidam nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
 */
#include "Python.h"
#include "structmember.h" /* Python include for object definition */
#include <ctemplate/template.h>

/* Error reporting for module init functions (adapted from mxProxy) */
#define Py_ReportModuleInitError(modname) {			\
    PyObject *exc_type, *exc_value, *exc_tb;			\
    PyObject *str_type, *str_value;				\
								\
    /* Fetch error objects and convert them to strings */	\
    PyErr_Fetch(&exc_type, &exc_value, &exc_tb);		\
    if (exc_type && exc_value) {				\
	str_type = PyObject_Str(exc_type);			\
	str_value = PyObject_Str(exc_value);			\
    }								\
    else {							\
	str_type = NULL;					\
	str_value = NULL;					\
    }								\
    /* Try to format a more informative error message using the	\
       original error */					\
    if (str_type && str_value &&				\
	PyString_Check(str_type) && PyString_Check(str_value))	\
	PyErr_Format(						\
		PyExc_ImportError,				\
		"initialization of module "modname" failed "	\
		"(%s:%s)",					\
		PyString_AS_STRING(str_type),			\
		PyString_AS_STRING(str_value));			\
    else							\
	PyErr_SetString(					\
		PyExc_ImportError,				\
		"initialization of module "modname" failed");	\
    Py_XDECREF(str_type);					\
    Py_XDECREF(str_value);					\
    Py_XDECREF(exc_type);					\
    Py_XDECREF(exc_value);					\
    Py_XDECREF(exc_tb);						\
}

// type convert int -> ctemplate::Strip
static ctemplate::Strip strip_from_int (unsigned int i) {
    switch (i) {
    case 0:
        return ctemplate::DO_NOT_STRIP;
    case 1:
        return ctemplate::STRIP_BLANK_LINES;
    case 2:
        return ctemplate::STRIP_WHITESPACE;
    default:
        return ctemplate::DO_NOT_STRIP;
    }
}

/*********************** Dictionary *************************/
/* Type definition */
typedef struct {
    PyObject_HEAD
    ctemplate::TemplateDictionary* dict;
    // Subdirectories don't have to be deleted on dealloc.
    bool subdict;
} Dictionary_Object;

/* create Dictionary object */
static PyObject*
Dictionary_New (PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Dictionary_Object* self;
    if ((self = (Dictionary_Object*) type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    self->dict = NULL;
    self->subdict = false;
    return (PyObject*)self;
}

/* initialize Dictionary object */
static int
Dictionary_Init (Dictionary_Object* self, PyObject* args) {
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return -1;
    self->dict = new ctemplate::TemplateDictionary(std::string(name));
    return 0;
}

/* dealloc Dictionary object */
static void
Dictionary_Dealloc (Dictionary_Object* self) {
    if (!self->subdict && self->dict) {
        delete self->dict;
        self->dict = NULL;
    }
    self->ob_type->tp_free((PyObject*)self);
}

/* Dictionary.SetValue(name, value) -> None */
static PyObject*
Dictionary_SetValue (Dictionary_Object* self, PyObject* args) {
    const char* name;
    PyObject* value;
    PyObject* strvalue;
    const char* cvalue;
    if (!PyArg_ParseTuple(args, "sO", &name, &value))
        return NULL;
    // increment ref for PyObject_Str call
    Py_INCREF(value);
    if ((strvalue = PyObject_Str(value)) == NULL) {
        Py_DECREF(value);
        return NULL;
    }
    Py_DECREF(value);
    if ((cvalue = PyString_AsString(strvalue)) == NULL) {
        Py_DECREF(strvalue);
        return NULL;
    }
    self->dict->SetValue(ctemplate::TemplateString(name),
                         ctemplate::TemplateString(cvalue));
    // XXX When strvalue is garbage collected, the internal char buffer
    // is freed, which invalidates the TemplateString buffer.
    // But then the strvalue ref is never decremented.
    // Py_DECREF(strvalue);
    Py_RETURN_NONE;
}

/* Dictionary.ShowSection(name) -> None */
static PyObject*
Dictionary_ShowSection (Dictionary_Object* self, PyObject* args) {
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    self->dict->ShowSection(ctemplate::TemplateString(name));
    Py_RETURN_NONE;
}

/* Dictionary.SetValueAndShowSection(name, value, section) -> None */
static PyObject*
Dictionary_SetValueAndShowSection (Dictionary_Object* self, PyObject* args) {
    const char* name;
    PyObject* value;
    PyObject* strvalue;
    const char* cvalue;
    const char* section;
    if (!PyArg_ParseTuple(args, "sOs", &name, &value, &section))
        return NULL;
    // increment ref for PyObject_Str call
    Py_INCREF(value);
    if ((strvalue = PyObject_Str(value)) == NULL) {
        Py_DECREF(value);
        return NULL;
    }
    Py_DECREF(value);
    if ((cvalue = PyString_AsString(strvalue)) == NULL) {
        Py_DECREF(strvalue);
        return NULL;
    }
    self->dict->SetValueAndShowSection(ctemplate::TemplateString(name),
                                       ctemplate::TemplateString(cvalue),
                                       ctemplate::TemplateString(section));
    // XXX When strvalue is garbage collected, the internal char buffer
    // is freed, which invalidates the TemplateString buffer.
    // But then the strvalue ref is never decremented.
    // Py_DECREF(strvalue);
    Py_RETURN_NONE;
}

/* Dictionary.Dump() -> String */
static PyObject*
Dictionary_Dump (Dictionary_Object* self, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    std::string out;
    self->dict->DumpToString(&out);
    return PyString_FromStringAndSize(out.c_str(), out.length());
}

extern PyTypeObject Dictionary_Type;

/* Dictionary.AddSectionDictionary(name) -> Dictionary */
static PyObject*
Dictionary_AddSectionDictionary (Dictionary_Object* self, PyObject* args) {
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    // extern PyTypeObject Dictionary_Type;
    PyTypeObject* type = &Dictionary_Type;
    Dictionary_Object* dict;
    if ( (dict = (Dictionary_Object*) type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    dict->subdict = true;
    dict->dict = self->dict->
        AddSectionDictionary(ctemplate::TemplateString(name));
    return (PyObject*)dict;
}

/* Dictionary.AddIncludeDictionary(name) -> Dictionary */
static PyObject*
Dictionary_AddIncludeDictionary (Dictionary_Object* self, PyObject* args) {
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    // extern PyTypeObject Dictionary_Type;
    PyTypeObject* type = &Dictionary_Type;
    Dictionary_Object* dict;
    if ( (dict = (Dictionary_Object*) type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    dict->subdict = true;
    dict->dict = self->dict->
        AddIncludeDictionary(ctemplate::TemplateString(name));
    return (PyObject*)dict;
}

/* Dictionary.SetFilename(name) -> None */
static PyObject*
Dictionary_SetFilename (Dictionary_Object* self, PyObject* args) {
    const char* name;
    size_t name_len;
    if (!PyArg_ParseTuple(args, "s#", &name, &name_len))
        return NULL;
    self->dict->SetFilename(ctemplate::TemplateString(name, name_len));
    Py_RETURN_NONE;
}

static PyObject *
Dictionary_SetGlobalValue (Dictionary_Object* self, PyObject* args) {
    const char* name;
    PyObject* value;
    PyObject* strvalue;
    const char* cvalue;
    if (!PyArg_ParseTuple(args, "sO", &name, &value))
        return NULL;
    Py_INCREF(value);
    if ((strvalue = PyObject_Str(value)) == NULL) {
        Py_DECREF(value);
        return NULL;
    }
    Py_DECREF(value);
    if ((cvalue = PyString_AsString(strvalue)) == NULL) {
        Py_DECREF(strvalue);
        return NULL;
    }
    self->dict->
        SetTemplateGlobalValue(ctemplate::TemplateString(name),
                               ctemplate::TemplateString(cvalue));
    // XXX When strvalue is garbage collected, the internal char buffer
    // is freed, which invalidates the TemplateString buffer.
    // But then the strvalue ref is never decremented.
    // Py_DECREF(strvalue);
    Py_RETURN_NONE;
}

static PyMethodDef Dictionary_Methods[] = {
    {"SetValue", (PyCFunction)Dictionary_SetValue, METH_VARARGS,
     "Set variable value."},
    {"ShowSection", (PyCFunction)Dictionary_ShowSection, METH_VARARGS,
     "Show section."},
    {"SetValueAndShowSection", (PyCFunction)Dictionary_SetValueAndShowSection, METH_VARARGS,
     "If value is non-empty, add a single single dictionary to\n"
     "section_name and call section_dict->AddValue(name, value)."},
    {"Dump", (PyCFunction)Dictionary_Dump, METH_VARARGS,
     "Dump to string."},
    {"AddSectionDictionary", (PyCFunction)Dictionary_AddSectionDictionary,
     METH_VARARGS, "Add section dictionary."},
    {"AddIncludeDictionary", (PyCFunction)Dictionary_AddIncludeDictionary,
     METH_VARARGS, "Add include dictionary."},
    {"SetFilename", (PyCFunction)Dictionary_SetFilename, METH_VARARGS,
     "Set filename."},
    {"SetGlobalValue", (PyCFunction)Dictionary_SetGlobalValue, METH_VARARGS,
     "This is used for a value that you want to be 'global', but only\n"
     "in the scope of a given template, including all its sections and\n"
     "all its sub-included dictionaries.  The main difference between\n"
     "SetGlobalValue() and SetValue(), is that SetGlobalValue()\n"
     "values persist across template-includes."},
    {NULL} /* Sentinel */
};

/*
 a) Dictionary[name] = value == Dictionary.SetValue(name, value)
 b) Dictionary[name] = True  == Dictionary.ShowSection(name)
 */
static int
dict_ass_sub(Dictionary_Object* self, PyObject *name, PyObject *value) {
    if (value == NULL) {
        PyErr_Format(PyExc_AttributeError, "deletion of values not supported");
        return -1;
    }
    const char* cname;
    if ((cname = PyString_AsString(name)) == NULL) {
        return -1;
    }
    Py_INCREF(value);
    if (PyBool_Check(value)) {
        int res;
        if ((res = PyObject_IsTrue(value)) == -1) {
            Py_DECREF(value);
            return -1;
        }
        if (res) {
            self->dict->ShowSection(ctemplate::TemplateString(cname));
        }
    }
    else
    {
        char* cvalue;
        PyObject* strvalue;
        if ((strvalue = PyObject_Str(value)) == NULL) {
            Py_DECREF(value);
            return -1;
        }
        if ((cvalue = PyString_AsString(strvalue)) == NULL) {
            Py_DECREF(value);
            Py_DECREF(strvalue);
            return -1;
        }
        self->dict->SetValue(ctemplate::TemplateString(cname),
                             ctemplate::TemplateString(cvalue));
    }
    Py_DECREF(value);
    // XXX When strvalue is garbage collected, the internal char buffer
    // is freed, which invalidates the TemplateString buffer.
    // But then the strvalue ref is never decremented.
    // Py_DECREF(strvalue);
    return 0;
}

static PyMappingMethods dict_as_mapping = {
    0, /* mp_length */
    0, /* mp_subscript */
    (objobjargproc)dict_ass_sub, /* mp_ass_subscript */
};

PyTypeObject Dictionary_Type = {
    PyObject_HEAD_INIT(NULL)
    0,              /* ob_size */
    "ctemplate.Dictionary",             /* tp_name */
    sizeof(Dictionary_Object), /* tp_size */
    0,              /* tp_itemsize */
    /* methods */
    (destructor)Dictionary_Dealloc, /* tp_dealloc */
    0,              /* tp_print */
    0,              /* tp_getattr */
    0,              /* tp_setattr */
    0,              /* tp_compare */
    0,              /* tp_repr */
    0,              /* tp_as_number */
    0,              /* tp_as_sequence */
    &dict_as_mapping, /* tp_as_mapping */
    0,              /* tp_hash */
    0,              /* tp_call */
    0,              /* tp_str */
    0,              /* tp_getattro */
    0,              /* tp_setattro */
    0,              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    "maps names (as found in template files) to their values.\n"
    "There are three types of names:\n"
    "  variables: value is a string.\n"
    "  sections: value is a list of sub-dicts to use when expanding\n"
    "    the section; the section is expanded once per sub-dict.\n"
    "  template-include: value is a list of pairs: name of the template\n"
    "    file to include, and the sub-dict to use when expanding it.\n"
    "The object has routines for setting these values.", /* tp_doc */
    0,              /* tp_traverse */
    0,              /* tp_clear */
    0,              /* tp_richcompare */
    0,              /* tp_weaklistoffset */
    0,              /* tp_iter */
    0,              /* tp_iternext */
    Dictionary_Methods, /* tp_methods */
    0,              /* tp_members */
    0,              /* tp_getset */
    0,              /* tp_base */
    0,              /* tp_dict */
    0,              /* tp_descr_get */
    0,              /* tp_descr_set */
    0,              /* tp_dictoffset */
    (initproc)Dictionary_Init,  /* tp_init */
    0,              /* tp_alloc */
    Dictionary_New, /* tp_new */
    0,              /* tp_free */
    0,              /* tp_is_gc */
    0,              /* tp_bases */
    0,              /* tp_mro */
    0,              /* tp_cache */
    0,              /* tp_subclasses */
    0,              /* tp_weaklist */
    0,              /* tp_del */
};


/**************************** Template ******************************/

typedef struct {
    PyObject_HEAD
    ctemplate::Template* ctemplate;
} Template_Object;


/* create Template object */
static PyObject*
Template_New (PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Template_Object* self;
    if ((self = (Template_Object*) type->tp_alloc(type, 0)) == NULL) {
        return NULL;
    }
    self->ctemplate = NULL;
    return (PyObject*)self;
}

/* initialize Template object */
static int
Template_Init (Template_Object* self, PyObject* args) {
    PyObject* filename;
    int strip;
    if (!PyArg_ParseTuple(args, "Si", &filename, &strip))
        return -1;
    const char* cfilename = PyString_AsString(filename);
    self->ctemplate = ctemplate::Template::GetTemplate(std::string(cfilename),
                                                    strip_from_int(strip));
    // raise IOError when template filename was not readable
    if (self->ctemplate == NULL) {
        PyErr_Format(PyExc_IOError, "non-existing or unreadable file `%s'",
                     cfilename);
        return -1;
    }
    return 0;
}

/* deallocate Template object */
static void
Template_Dealloc (Template_Object* self) {
    // note that self->ctemplate will be deleted by ctemplate::ClearCache()
    self->ob_type->tp_free((PyObject*)self);
}

/* Template.Expand(dict) -> String */
static PyObject*
Template_Expand (Template_Object* self, PyObject* args) {
    Dictionary_Object* dict;
    if (!PyArg_ParseTuple(args, "O", &dict))
        return NULL;
    std::string output;
    self->ctemplate->Expand(&output, dict->dict);
    return PyString_FromStringAndSize(output.c_str(), output.length());
}

/* Template.state() -> int */
static PyObject*
Template_State (Template_Object* self, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return PyInt_FromLong(self->ctemplate->state());
}

/* Template.ReloadIfChanged() -> bool */
static PyObject*
Template_ReloadIfChanged (Template_Object* self, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return PyBool_FromLong(self->ctemplate->ReloadIfChanged());
}

static PyMethodDef Template_Methods[] = {
    {"Expand", (PyCFunction)Template_Expand, METH_VARARGS,
    "Expands the template into a string using the values\n"
    "in the supplied dictionary."},
    {"ReloadIfChanged", (PyCFunction)Template_ReloadIfChanged, METH_VARARGS,
    "Reloads the file from the filesystem iff its mtime is different\n"
    "now from what it was last time the file was reloaded.  Note a\n"
    "file is always \"reloaded\" the first time this is called.  If\n"
    "the file is in fact reloaded, then the contents of the file are\n"
    "parsed into the template node parse tree by calling BuildTree\n"
    "After this call, the state of the Template will be either\n"
    "TS_READY or TS_ERROR.  Return value is true iff we reloaded\n"
    "because the content changed and could be parsed with no errors."},
    {"state", (PyCFunction)Template_State, METH_VARARGS,
    "One of TS_EMPTY, TS_ERROR, TS_READY, TS_RELOAD"},
    {NULL} /* Sentinel */
};

static PyTypeObject Template_Type = {
    PyObject_HEAD_INIT(NULL)
    0,              /* ob_size */
    "ctemplate.Template",             /* tp_name */
    sizeof(Template_Object), /* tp_size */
    0,              /* tp_itemsize */
    /* methods */
    (destructor)Template_Dealloc, /* tp_dealloc */
    0,              /* tp_print */
    0,              /* tp_getattr */
    0,              /* tp_setattr */
    0,              /* tp_compare */
    0,              /* tp_repr */
    0,              /* tp_as_number */
    0,              /* tp_as_sequence */
    0,              /* tp_as_mapping */
    0,              /* tp_hash */
    0,              /* tp_call */
    0,              /* tp_str */
    0,              /* tp_getattro */
    0,              /* tp_setattro */
    0,              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    "Object which reads and parses the template file and then is used to\n"
    "expand the parsed structure to a string.", /* tp_doc */
    0,              /* tp_traverse */
    0,              /* tp_clear */
    0,              /* tp_richcompare */
    0,              /* tp_weaklistoffset */
    0,              /* tp_iter */
    0,              /* tp_iternext */
    Template_Methods, /* tp_methods */
    0,              /* tp_members */
    0,              /* tp_getset */
    0,              /* tp_base */
    0,              /* tp_dict */
    0,              /* tp_descr_get */
    0,              /* tp_descr_set */
    0,              /* tp_dictoffset */
    (initproc)Template_Init,  /* tp_init */
    0,              /* tp_alloc */
    Template_New, /* tp_new */
    0,              /* tp_free */
    0,              /* tp_is_gc */
    0,              /* tp_bases */
    0,              /* tp_mro */
    0,              /* tp_cache */
    0,              /* tp_subclasses */
    0,              /* tp_weaklist */
    0,              /* tp_del */
};

static PyObject *
ctemplate_SetGlobalValue (PyObject* self, PyObject* args) {
    const char* name;
    size_t name_len;
    PyObject* obj;
    if (!PyArg_ParseTuple(args, "s#O", &name, &name_len, &obj))
        return NULL;
    char* value;
    int value_len;
    PyObject* value_obj = PyObject_Str(obj);
    if (value_obj == NULL) {
        Py_DECREF(obj);
        return NULL;
    }
    if (PyString_AsStringAndSize(value_obj, &value, &value_len) == -1) {
        Py_DECREF(obj);
        Py_DECREF(value_obj);
        return NULL;
    }
    ctemplate::TemplateDictionary::
        SetGlobalValue(ctemplate::TemplateString(name, name_len),
                       ctemplate::TemplateString(value, value_len));
    Py_DECREF(obj);
    Py_DECREF(value_obj);
    Py_RETURN_NONE;
}

static PyObject *
ctemplate_SetTemplateRootDirectory (PyObject* self, PyObject* args) {
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    return PyBool_FromLong(ctemplate::Template::
                           SetTemplateRootDirectory(std::string(name)));
}

static PyObject *
ctemplate_GetTemplateRootDirectory (PyObject* self, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    std::string name = ctemplate::Template::template_root_directory();
    return PyString_FromString(name.c_str());
}

static PyObject *
ctemplate_ReloadAllIfChanged (PyObject* self, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    ctemplate::Template::ReloadAllIfChanged();
    Py_RETURN_NONE;
}

static PyObject *
ctemplate_ClearCache (PyObject* self, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    ctemplate::Template::ClearCache();
    Py_RETURN_NONE;
}

static PyObject *
ctemplate_RegisterTemplate (PyObject* self, PyObject* args) {
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    ctemplate::TemplateNamelist::RegisterTemplate(name);
    Py_RETURN_NONE;
}

static PyObject*
ctemplate_GetBadSyntaxList (PyObject* self, PyObject* args) {
    unsigned int i;
    PyObject* obj;
    if (!PyArg_ParseTuple(args, "OI", &obj, &i))
        return NULL;
    Py_INCREF(obj);
    int refresh;
    if ((refresh = PyObject_IsTrue(obj)) == -1) {
        Py_DECREF(obj);
        return NULL;
    }
    Py_DECREF(obj);
    ctemplate::TemplateNamelist::SyntaxListType the_list =
        ctemplate::TemplateNamelist::
        GetBadSyntaxList((refresh==1), strip_from_int(i));
    PyObject* pylist;
    if ((pylist = PyList_New(the_list.size())) == NULL) {
        return NULL;
    }
    // construct list
    for (unsigned int i=0; i < the_list.size(); i++) {
        if ((obj = PyString_FromString(the_list[i].c_str())) == NULL) {
            Py_DECREF(pylist);
            return NULL;
        }
        if (PyList_SetItem(pylist, i, obj) == -1) {
            Py_DECREF(pylist);
            return NULL;
        }
    }
    return pylist;
}

static PyMethodDef ctemplate_methods[] = {
    {"SetGlobalValue", (PyCFunction)ctemplate_SetGlobalValue, METH_VARARGS,
    "Set global variable value."},
    {"SetTemplateRootDirectory", (PyCFunction)ctemplate_SetTemplateRootDirectory, METH_VARARGS,
     "Sets the root directory for all templates used by the program.\n"
     "After calling this method, the filename passed to GetTemplate\n"
     "may be a relative pathname (no leading '/'), in which case\n"
     "this root-directory is prepended to the filename.\n"},
    {"GetTemplateRootDirectory", (PyCFunction)ctemplate_GetTemplateRootDirectory, METH_VARARGS,
     "Returns the stored template root directory name"},
    {"ReloadAllIfChanged", (PyCFunction)ctemplate_ReloadAllIfChanged,
     METH_VARARGS,
     "Marks each template object in the cache to check to see if\n"
     "its template file has changed the next time it is invoked\n"
     "via GetTemplate. If it finds the file has changed, it\n"
     "then reloads and parses the file before being returned by\n"
     "GetTemplate."},
    {"ClearCache", (PyCFunction)ctemplate_ClearCache, METH_VARARGS,
     "Deletes all the template objects in the cache. This should only\n"
     "be done once, just before exiting the program and after all\n"
     "template expansions are completed. (If you want to refresh the\n"
     "cache, the correct method to use is ReloadAllIfChanged, not\n"
     "this one.) Note: this method is not necessary unless you are\n"
     "testing for memory leaks. Calling this before exiting the\n"
     "program will prevent unnecessary reporting in that case."},
     {"RegisterTemplate", (PyCFunction)ctemplate_RegisterTemplate, METH_VARARGS,
      "Takes a name and pushes it onto the static namelist."},
     {"GetBadSyntaxList", (PyCFunction)ctemplate_GetBadSyntaxList, METH_VARARGS,
      "If refresh is true or if it is the first time the function is called\n"
      "in the execution of the program, it creates (or clears) the 'bad\n"
      "syntax' list and then fills it with the list of\n"
      "templates that the program knows about but contain syntax errors.\n"
      "A missing file is not considered a syntax error, and thus is\n"
      "not included in this list.\n"
      "If refresh is false and it is not the first time the function is\n"
      "called, it merely returns the list created in the\n"
      "call when the last refresh was done.\n"
      "NOTE: The side effect of calling this the first time or\n"
      "with refresh equal true is that all templates are parsed and cached.\n"
      "Hence they need to be retrieved with the flags that\n"
      "the program needs them loaded with (i.e, the strip parameter\n"
      "passed to Template::GetTemplate.)."},
    {NULL} /* Sentinel */
};

static void
add_constants (PyObject* m) {
    PyModule_AddIntConstant(m, "DO_NOT_STRIP", ctemplate::DO_NOT_STRIP);
    PyModule_AddIntConstant(m, "STRIP_BLANK_LINES", ctemplate::STRIP_BLANK_LINES);
    PyModule_AddIntConstant(m, "STRIP_WHITESPACE", ctemplate::STRIP_WHITESPACE);
    PyModule_AddIntConstant(m, "TS_EMPTY", ctemplate::TS_EMPTY);
    PyModule_AddIntConstant(m, "TS_ERROR", ctemplate::TS_ERROR);
    PyModule_AddIntConstant(m, "TS_READY", ctemplate::TS_READY);
    // PyModule_AddIntConstant(m, "TS_SHOULD_RELOAD", ctemplate::TS_SHOULD_RELOAD);
    PyModule_AddIntConstant(m, "TC_HTML", ctemplate::TC_HTML);
    PyModule_AddIntConstant(m, "TC_JS", ctemplate::TC_JS);
    PyModule_AddIntConstant(m, "TC_CSS", ctemplate::TC_CSS);
    PyModule_AddIntConstant(m, "TC_JSON", ctemplate::TC_JSON);
    PyModule_AddIntConstant(m, "TC_XML", ctemplate::TC_XML);
    PyModule_AddIntConstant(m, "TC_MANUAL", ctemplate::TC_MANUAL);
}

static void
clear_template_cache (void) {
    ctemplate::Template::ClearCache();
}

extern "C" {

static void
ctemplate_Cleanup (void) {
    clear_template_cache();
}

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
/* initialization of the module */
PyMODINIT_FUNC initctemplate (void) {
    PyObject* m;
    if (PyType_Ready(&Template_Type) < 0) {
        return;
    }
    if (PyType_Ready(&Dictionary_Type) < 0) {
        return;
    }
    m = Py_InitModule3("ctemplate", ctemplate_methods,
                       "Wrapper for the ctemplate library.");
    if (m == NULL) {
        goto onError;
    }
    /* Register cleanup function */
    if (Py_AtExit(ctemplate_Cleanup) == -1)
        goto onError;

    Py_INCREF(&Template_Type);
    if (PyModule_AddObject(m, "Template",
                           (PyObject *)&Template_Type) == -1) {
        goto onError;
    }
    Py_INCREF(&Dictionary_Type);
    if (PyModule_AddObject(m, "Dictionary",
                           (PyObject *)&Dictionary_Type) == -1) {
        goto onError;
    }
    add_constants(m);
onError:
    if (PyErr_Occurred())
	Py_ReportModuleInitError("ctemplate");
}

} /* extern C */
