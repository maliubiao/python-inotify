#include <Python.h> 
#include <structmember.h>
#include <sys/types.h> 
#include <sys/inotify.h>
#include <sys/epoll.h>


/* module globals */
int inotify_fd = 0;
int epoll_fd = 0; 
int inotifyes = 0; 
int stop_loop = 0;
void *inotify_eb = NULL; 
struct epoll_event *epolles = NULL; 
PyObject *inotify_callback = NULL; 


/* inotify event object */
typedef struct {
	PyObject_HEAD 
	PyObject *wd;
	PyObject *mask;
	PyObject *cookie;
	PyObject *length;
	PyObject *name; 
} INOTIFYEVENTobject;	


static PyTypeObject INOTIFYEVENTtype; 


static int init_inotify()
{
	int tmp = 0;
	if(inotify_fd == 0)	{
		tmp = inotify_init1(IN_NONBLOCK);	
		if (tmp > 0) {
			inotify_fd = tmp;
			return 0;
		}
		return -1;
	} 
	return 0; 
}
		
			
static int init_epoll()
{
	int tmp = 0;
	if(epoll_fd == 0)	{
		tmp = epoll_create(1);
		if (tmp > 0) { 
			epoll_fd = tmp;
			return 0;
		}
		return -1;
	}
	return 0; 

}


static int notify_client(struct inotify_event *ie) {
	INOTIFYEVENTobject *ietmp;
	PyObject *ietmp_args;

	ietmp = (INOTIFYEVENTobject *)PyObject_New(
						INOTIFYEVENTobject, 
						&INOTIFYEVENTtype);
	ietmp->wd = PyInt_FromLong(ie->wd);
	ietmp->mask = PyInt_FromLong(ie->mask);
	ietmp->cookie = PyInt_FromLong(ie->cookie);
	ietmp->length = PyInt_FromLong(ie->len);
	ietmp->name = PyString_FromString(ie->name);
	ietmp_args = Py_BuildValue("(O)", ietmp); 
	PyObject_CallObject(inotify_callback, ietmp_args); 
	if (PyErr_Occurred()) {
		return -1;
	}
	Py_DECREF(ietmp_args); 
	Py_DECREF(ietmp);
	return 0;

}


PyDoc_STRVAR(inotify_watch_doc,
		"add the file yout want to watch, this function accepts"
		"filepath, mask as parameters and  return a "
		"fd which you can use to identify yout file.)\n");


static PyObject *
inotify_watch(PyObject *object, PyObject *args)
{ 
	PyObject *mask;
	PyObject *name;
	int tmp = 0 ;
	if (!PyArg_ParseTuple(args, "OO:watch", &name, &mask))
		return NULL;

	if (!PyInt_Check(mask)) {
		PyErr_SetString(PyExc_TypeError,
				"mask should be a integer");
		return NULL;
	}

	if (!PyString_Check(name)) {
		PyErr_SetString(PyExc_TypeError, 
				"name should be a string");
		return NULL;
	} 

	if (init_inotify() != 0) {
		PyErr_SetString(PyExc_RuntimeError,
				"init inotify instance failed");
		return NULL;
	}

	tmp = inotify_add_watch(inotify_fd, 
			PyString_AsString(name),
			PyInt_AsLong(mask));
	if(tmp < 0) {
		PyErr_SetFromErrno(PyExc_RuntimeError);
		return NULL; 
	}

	if (PyErr_Occurred()) {
		return NULL;
	}
	return (PyObject *)PyInt_FromLong(tmp); 
}


PyDoc_STRVAR(inotify_unwatch_doc,
		"stop watching a file,  this function accepts a fd"
		"as parameter, which is returned by function watch.\n");


static PyObject *
inotify_unwatch(PyObject *object, PyObject *args)
{
	int tmp = 0;
	int wd = 0;
	if(!PyArg_ParseTuple(args, "i:unwatch", &wd))
		return NULL;

	if (init_inotify() != 0) {
		PyErr_SetString(PyExc_RuntimeError,
				"create inotify instance failed");
		return NULL;
	}

	tmp = inotify_rm_watch(inotify_fd, wd);	
	if(tmp < 0) {
		PyErr_SetFromErrno(PyExc_RuntimeError);
		return NULL;
	} 

	Py_RETURN_TRUE;
}


PyDoc_STRVAR(inotify_startloop_doc,
		"start the mainloop, this function accepts a callable"
		"object as parameter and return nothing, there is an"
		"optional parameter extra_code for you to add something"
		"to the mainloop, \n");


static PyObject *
inotify_startloop(PyObject *object, PyObject *args, PyObject *kwargs)
{
	int tmp = 0;
	int mfds = 0;
	int m = 0;
	int n = 0;
	struct epoll_event ev; 
	PyObject *cb_tmp = NULL;
	PyObject *extra_tmp = NULL;
	static char *kwlist[] = {"extra_code", 0};

	if (PyArg_ParseTupleAndKeywords(
				args, kwargs, "O|O:startloop", kwlist, 
				&cb_tmp, &extra_tmp)) {
				
		if (!PyCallable_Check(cb_tmp)) {
			PyErr_SetString(PyExc_TypeError,
					"parameter should be a callable"
					"object");
				return NULL;
		} 
		if (extra_tmp != NULL) {
			if (!PyCallable_Check(extra_tmp)) {
				PyErr_SetString(PyExc_TypeError, 
						"extra_code should be "
						"a callable object");
					return NULL;
			}
		}

		Py_XINCREF(cb_tmp);
		Py_XDECREF(inotify_callback);
		inotify_callback = cb_tmp;
	}	

	if (init_inotify() != 0) {
		PyErr_SetString(PyExc_RuntimeError,
				"inotify has not been initalized");
		return NULL;
	}

	ev.events = EPOLLIN;
	ev.data.fd = inotify_fd;

	if (init_epoll() != 0) {
		PyErr_SetString(PyExc_RuntimeError, 
				"create epoll instance failed");
		return NULL;
	}

	tmp = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &ev);
	if(tmp < 0) {
		PyErr_SetFromErrno(PyExc_RuntimeError);
		return NULL;
	} 
	stop_loop = 1;

	for (;;) {
		if (stop_loop == 0) {
			close(epoll_fd);
			epoll_fd = 0;
			Py_RETURN_NONE;
		}
		mfds = epoll_wait(epoll_fd, epolles, 10, -1);
		if (mfds < 0) {
			PyErr_SetFromErrno(PyExc_RuntimeError);
			return NULL;
		}
		for (m; m < mfds; ++m) {
			n = read(inotify_fd,
				inotify_eb,
				inotifyes);
			if (n == EAGAIN)
				continue;
			if (n < 0) {
				PyErr_SetFromErrno(PyExc_RuntimeError);
				return NULL;
			}
			if (notify_client((struct inotify_event *)
					inotify_eb) < 0) {
				return NULL;
			}
		} 
		if (extra_tmp != NULL) {
			Py_INCREF(Py_None);		
			PyObject_CallObject(extra_tmp, Py_None);
			Py_DECREF(Py_None);
		}
		m = 0;
		
	}
	Py_RETURN_TRUE;
	
}


PyDoc_STRVAR(inotify_stoploop_doc, 
		"stop the main loop\n");
	    
		
static PyObject *
inotify_stoploop(PyObject *object, PyObject *args)
{
	stop_loop = 0;
	close(epoll_fd);
	epoll_fd = 0; 
	Py_RETURN_NONE;
}


static PyMethodDef INOTIFY_methods[] = { 
	{"watch", (PyCFunction)inotify_watch,
		METH_VARARGS, inotify_watch_doc},
	{"unwatch", (PyCFunction)inotify_unwatch,
		METH_VARARGS, inotify_unwatch_doc},
	{"startloop", (PyCFunction)inotify_startloop,
		METH_VARARGS, inotify_startloop_doc},
	{"stoploop", (PyCFunction)inotify_stoploop,
		METH_NOARGS, inotify_stoploop_doc},
	{NULL, NULL, 0, NULL}
};


static PyMemberDef INOTIFYEVENT_members[] = {
	{"wd", T_OBJECT, offsetof(INOTIFYEVENTobject, wd),
		READONLY, "watch descriptor"},
	{"mask", T_OBJECT, offsetof(INOTIFYEVENTobject, mask),
		READONLY, "event mask"},
	{"cookie", T_OBJECT, offsetof(INOTIFYEVENTobject, cookie),
		READONLY, "event signature"},
	{"length", T_OBJECT, offsetof(INOTIFYEVENTobject, length),
		READONLY, "name length"},
	{"name", T_OBJECT, offsetof(INOTIFYEVENTobject, name),
		READONLY, "optional name"},
	{NULL}
};


static void 
INOTIFYEVENT_dealloc(INOTIFYEVENTobject* self)
{ 
	Py_CLEAR(self->wd);
	Py_CLEAR(self->mask);
	Py_CLEAR(self->cookie);
	Py_CLEAR(self->length);
	Py_CLEAR(self->name);
	PyObject_Del(self);
}

static PyObject* 
INOTIFYEVENT_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	INOTIFYEVENTobject *self;		
	self = PyObject_New(INOTIFYEVENTobject, type);
	if (!self)
		return NULL;
	self->wd = PyInt_FromLong(0);	
	if (!self->wd) {
		Py_DECREF(self);
		return NULL;
	}
	self->mask = PyInt_FromLong(0);
	if (!self->mask) {
		Py_DECREF(self);
		return NULL;
	}
	self->cookie = PyInt_FromLong(0);		
	if (!self->cookie) {
		Py_DECREF(self);
		return NULL;
	}
	self->length = PyInt_FromLong(0);
	if (!self->length) {
		Py_DECREF(self);
		return NULL;
	}
	self->name = PyString_FromString("");
	if (!self->name) {
		Py_DECREF(self);
		return NULL;
	} 
	return (PyObject *)self;
}


PyDoc_STRVAR(event_doc, 
		"document: man inotify -> struct inotify_event"); 
		


static PyTypeObject INOTIFYEVENTtype = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"inotify.event",
	sizeof(INOTIFYEVENTobject),
	0,
	/* methods */
	(destructor)INOTIFYEVENT_dealloc,	/*tp_dealloc*/
	0,			/*tp_print*/
	0,			/*tp_getattr*/
	0,			/*tp_setattr*/
	0,			/*tp_compare*/
	0,			/*tp_repr*/
	0,			/*tp_as_number*/
	0,			/*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	0,			/*tp_hash*/
	0,			/*tp_call*/
	0,			/*tp_str*/
	0,			/*tp_getattro*/
	0,			/*tp_setattro*/
	0,			/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,	/*tp_flags*/
	event_doc,		/*tp_doc*/
	0,			/*tp_traverse*/
	0,			/*tp_clear*/
	0,			/*tp_richcompare*/
	0,			/*tp_weaklistoffset*/
	0,			/*tp_iter*/
	0,			/*tp_iternext*/
	0,			/*tp_methods*/
	INOTIFYEVENT_members,	/*tp_members*/
	0,			/*tp_getset*/
	0,			/*tp_base*/
	0,			/*tp_dict*/
	0,			/*tp_descr_get*/
	0,			/*tp_descr_set*/
	0,			/*tp_dictoffset*/
	0,			/*tp_init*/
	0,			/*tp_alloc*/
	INOTIFYEVENT_new,	/*tp_new*/
};



PyMODINIT_FUNC initinotify(void)
{
	PyObject *m;
	Py_TYPE(&INOTIFYEVENTtype) = &PyType_Type;
	if (PyType_Ready(&INOTIFYEVENTtype) < 0)
		return;
	m  = Py_InitModule("pyinotify", INOTIFY_methods);
	if (m == NULL)
		return; 
	Py_INCREF((PyObject *) &INOTIFYEVENTtype);
	PyModule_AddObject(m, "event", (PyObject *) &INOTIFYEVENTtype);
	/*inotify consts*/
	PyModule_AddObject(m, "IN_ACCESS", PyInt_FromLong(IN_ACCESS));
	PyModule_AddObject(m, "IN_ATTRIB", PyInt_FromLong(IN_ATTRIB));
	PyModule_AddObject(m, "IN_CREATE", PyInt_FromLong(IN_CREATE));
	PyModule_AddObject(m, "IN_DELETE", PyInt_FromLong(IN_DELETE));
	PyModule_AddObject(m, "IN_MODIFY", PyInt_FromLong(IN_MODIFY));
	PyModule_AddObject(m, "IN_MOVE_SELF", PyInt_FromLong(IN_MOVE_SELF));
	PyModule_AddObject(m, "IN_MOVED_TO", PyInt_FromLong(IN_MOVED_TO));
	PyModule_AddObject(m, "IN_OPEN", PyInt_FromLong(IN_OPEN));
	PyModule_AddObject(m, "IN_MASK_ADD", PyInt_FromLong(IN_MASK_ADD));
	PyModule_AddObject(m, "IN_ONESHOT", PyInt_FromLong(IN_ONESHOT));
	PyModule_AddObject(m, "IN_ONLYDIR", PyInt_FromLong(IN_ONLYDIR));
	PyModule_AddObject(m, "IN_IGNORED", PyInt_FromLong(IN_IGNORED));
	PyModule_AddObject(m, "IN_ISDIR", PyInt_FromLong(IN_ISDIR));
	PyModule_AddObject(m, "IN_UNMOUNT", PyInt_FromLong(IN_UNMOUNT)); 
	PyModule_AddObject(m, "IN_CLOSE_WRITE",
			PyInt_FromLong(IN_CLOSE_WRITE));
	PyModule_AddObject(m, "IN_CLOSE_NOWRITE",
			PyInt_FromLong(IN_CLOSE_NOWRITE)); 
	PyModule_AddObject(m, "IN_DELETE_SELF",
			PyInt_FromLong(IN_DELETE_SELF));
	PyModule_AddObject(m, "IN_MOVED_FROM",
			PyInt_FromLong(IN_MOVED_FROM));
	PyModule_AddObject(m, "IN_ALL_EVENTS", 
			PyInt_FromLong(IN_ALL_EVENTS)); 
	PyModule_AddObject(m, "IN_DONT_FOLLOW",
			PyInt_FromLong(IN_DONT_FOLLOW));
	PyModule_AddObject(m, "IN_EXCL_UNLINK",
			PyInt_FromLong(IN_EXCL_UNLINK));
	PyModule_AddObject(m, "IN_Q_OVERFLOW",
			PyInt_FromLong(IN_Q_OVERFLOW)); 

	/*init module globals*/
	inotifyes = sizeof(struct inotify_event) + NAME_MAX + 1; 
	inotify_eb = PyMem_Malloc(sizeof(char) * inotifyes);
	epolles = PyMem_Malloc(sizeof(struct epoll_event) * 10);
}


