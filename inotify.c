#include <Python.h>
#include <sys/inotify.h>
#include <sys/epoll.h>

PyDoc_STRVAR(inotify_create_doc, "initializes a new inotify instance and returns a file descriptor associated with a new inotify event queue");

static PyObject *
inotify_create(PyObject *object, PyObject *args)
{
	int ret;
	unsigned long flags = 0;
	if (!PyArg_ParseTuple(args, ":k", &flags)) {
		return NULL;
	}
	ret = inotify_init1(flags);
	if (ret < 0) {
		PyErr_SetFromErrno(PyExc_OSError);
		return NULL;
	}
	return PyInt_FromLong(ret); 
}

PyDoc_STRVAR(inotify_add_doc, "adds a new watch, or modifies an existing watch, for the file whose locaion is specified in pathname");

static PyObject *
inotify_add(PyObject *object, PyObject *args)
{ 
	int fd;
	char *path;
	unsigned long mask;
	int ret;

	if (!PyArg_ParseTuple(args, "Isk", &fd, &path, &mask)) {
		return NULL;
	}

	ret = inotify_add_watch(fd, path, mask);
	if (ret < 0) {
		PyErr_SetFromErrno(PyExc_OSError);
		return NULL;
	}
	return PyInt_FromLong(ret);
}

PyDoc_STRVAR(inotify_rm_doc, "removes the watch associated with the watch descriptor wd from the inotify instance associated with the file descriptor fd. Removing a watch causes an IGNORED event");

static PyObject *
inotify_rm(PyObject *object, PyObject *args)
{
	int ret;
	int fd;
	int wd;

	if (!PyArg_ParseTuple(args, "II", &fd, &wd)) {
		return NULL;
	}
	ret = inotify_rm_watch(fd, wd);
	if (ret < 0) {
		PyErr_SetFromErrno(PyExc_OSError);
		return NULL;
	}
	Py_RETURN_NONE;
}

PyDoc_STRVAR(inotify_read_event_doc, "read event from inotify fd");

static PyObject *
inotify_read_event(PyObject *object, PyObject *args)
{ 
	int fd; 
	int ret; 
	PyObject *callback;
	PyObject *event_dict; 
	struct inotify_event *event;

	if (!PyArg_ParseTuple(args, "IO", &fd, &callback)) {
		return NULL;
	}

	if (!PyFunction_Check(callback)) {
		PyErr_SetString(PyExc_TypeError, "callback must be a function");
		return NULL;
	} 

	event = PyMem_Malloc(sizeof(*event) + 513);
	if (!event) {
		goto failed;	
	}

<<<<<<< HEAD
	ret = read(fd, event, sizeof(*event)); 
=======
	while(1) {
		if (stoploop == 0) { 
			Py_RETURN_NONE;
		}
		mfds = epoll_wait(epoll_fd, epoll_event, 100, timeout);
		if (mfds < 0) {
			PyErr_SetFromErrno(PyExc_OSError);
			return NULL;
		}
		for (m=0; m < mfds; ++m) {
			errno = 0;
			n = read(inotify_fd, inotify_buffer, event_size);
			if (errno == EAGAIN)
				continue;
			if (n < 0) {
				PyErr_SetFromErrno(PyExc_OSError);
				return NULL;
			}
			if (notify_client((struct inotify_event *)\
						inotify_buffer) < 0\
					) {
				return NULL;
			}	
		} 
		if (extra) { 
			PyObject_CallObject(extra, NULL); 
		} 
		if (!block) {
			restore_loop = 1;
			break;
		}
		m = 0; 
	}
	Py_RETURN_NONE;
	
}
>>>>>>> 76ebaf22d5abc52597179c9cdae903844cf17eb7

	if (ret != sizeof(*event)) { 
		goto failed; 
	}

	/* callback args */
	event_dict = PyDict_New();
	PyDict_SetItemString(event_dict, "wd", PyInt_FromLong(event->wd));
	PyDict_SetItemString(event_dict, "mask", PyLong_FromUnsignedLong(event->mask));
	PyDict_SetItemString(event_dict, "cookie", PyLong_FromUnsignedLong(event->cookie));
	PyDict_SetItemString(event_dict, "len", PyLong_FromUnsignedLong(event->len));
	PyDict_SetItemString(event_dict, "name", PyString_FromString(event->name)); 

	PyMem_Free(event);
	/* invoke callback, ignore ret */
	PyObject_CallFunction(callback, "(O)", event_dict);
	
	Py_XDECREF(event_dict); 

	Py_RETURN_NONE; 

failed:
	PyMem_Free(event);
	PyErr_SetFromErrno(PyExc_OSError);
	return NULL; 
	
} 


static PyMethodDef inotify_methods[] = {
	{"add", (PyCFunction)inotify_add,
		METH_VARARGS, inotify_add_doc},
	{"remove", (PyCFunction)inotify_rm,
		METH_VARARGS, inotify_rm_doc},
	{"create", (PyCFunction)inotify_create,
		METH_VARARGS, inotify_create_doc},
	{"read_event", (PyCFunction)inotify_read_event,
		METH_VARARGS, inotify_read_event_doc},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_inotify(void)
{
	PyObject *m;
	m = Py_InitModule("_inotify", inotify_methods);
	if (m != NULL) {
#define OBJECT_ADD_ULONG(x, y, z) PyModule_AddObject(x, y, PyLong_FromUnsignedLong(z))
	/* Supported events suitable for MASK parameter of INOTIFY_ADD_WATCH*/
	OBJECT_ADD_ULONG(m, "ACCESS", IN_ACCESS);	
	OBJECT_ADD_ULONG(m, "ATTRIB", IN_ATTRIB); 
	OBJECT_ADD_ULONG(m, "CLOSE_EXEC", IN_CLOEXEC);
	OBJECT_ADD_ULONG(m, "CLOSE_WRITE", IN_CLOSE_WRITE);
	OBJECT_ADD_ULONG(m, "CLOSE_NOWRITE", IN_CLOSE_NOWRITE);
	OBJECT_ADD_ULONG(m, "CREATE", IN_CREATE);
	OBJECT_ADD_ULONG(m, "DELETE", IN_DELETE);
	OBJECT_ADD_ULONG(m, "DELETE_SELF", IN_DELETE_SELF);
	OBJECT_ADD_ULONG(m, "MODIFY", IN_MODIFY);
	OBJECT_ADD_ULONG(m, "MOVE_SELF", IN_MOVE_SELF);
	OBJECT_ADD_ULONG(m, "MOVED_FROM", IN_MOVED_FROM);
	OBJECT_ADD_ULONG(m, "MOVED_TO", IN_MOVED_TO);
	OBJECT_ADD_ULONG(m, "OPEN", IN_OPEN);
	OBJECT_ADD_ULONG(m, "ALL_EVENTS", IN_ALL_EVENTS);
	/* helper event */
	OBJECT_ADD_ULONG(m, "CLOSE", IN_CLOSE);
	OBJECT_ADD_ULONG(m, "MOVE", IN_MOVE);
	/* Special flags */ 
	OBJECT_ADD_ULONG(m, "DONT_FOLLOW", IN_DONT_FOLLOW);
	OBJECT_ADD_ULONG(m, "EXCL_UNLINK", IN_EXCL_UNLINK);
	OBJECT_ADD_ULONG(m, "MASK_ADD", IN_MASK_ADD);
	OBJECT_ADD_ULONG(m, "IN_ONESHOT", IN_ONESHOT);
	OBJECT_ADD_ULONG(m, "IN_ONLYDIR", IN_ONLYDIR);
	OBJECT_ADD_ULONG(m, "IGNORED", IN_IGNORED);
	OBJECT_ADD_ULONG(m, "ISDIR", IN_ISDIR);
	OBJECT_ADD_ULONG(m, "Q_OVERFLOW", IN_Q_OVERFLOW);
	OBJECT_ADD_ULONG(m, "UNMOUNT", IN_UNMOUNT);
#undef OBJECT_ADD_ULONG
	}
}

