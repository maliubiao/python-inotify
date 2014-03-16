#include <Python.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/types.h>

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

PyDoc_STRVAR(inotify_read_event_doc,
"read_event(fd, callback[, TIMEOUT])\n\
\n\
Read one or more event(s) from an inotify file descriptor.  Without a timeout,\n\
blocks until an event becomes available; when a floating-point value is given\n\
for TIMEOUT, waits up to that many seconds for an event to become available.");

static PyObject *
inotify_read_event(PyObject *object, PyObject *args)
{
	int fd = -1;
	int ret;
	fd_set set;
	float timeout_arg = -1;
	struct timeval timeout = { 0, 0 };
	char* buffer = NULL;
	unsigned int avail;
	PyObject *callback;
	PyObject *event_dict;

	if (!PyArg_ParseTuple(args, "IO|f", &fd, &callback, &timeout_arg)) {
		return NULL;
	}

	if (!PyFunction_Check(callback)) {
		PyErr_SetString(PyExc_TypeError, "callback must be a function");
		return NULL;
	} 

	if ( timeout_arg > 0 ) {
	    /* Convert floating-point timeout value to `struct timeval`. */
	    float timeout_fraction = fmodf(timeout_arg, 1.0f);
	    timeout.tv_sec = (long int) (timeout_arg - timeout_fraction);
	    timeout.tv_usec = (long int) (timeout_fraction * 1000000);
	}

	/* Wait for one or more events to become available. */
	FD_ZERO(&set);
	FD_SET(fd, &set);

	if ( timeout_arg > 0 ) {
	    ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
	} else {
	    ret = select(FD_SETSIZE, &set, NULL, NULL, NULL);
	}

	if ( ret < 0 ) {
	    goto failed;
	} else if ( ret > 0 ) {
	    /* Find out how many bytes are in the queue. */
	    if ( ioctl(fd, FIONREAD, &avail) < 0 ) {
		goto failed;
	    }

	    /* Read all available events. */
	    if ( ! (buffer = PyMem_Malloc(avail)) ) {
		goto failed;
	    }
	    if ( read(fd, buffer, avail) < 0 ) {
		goto failed;
	    }

	    /* Dispatch the handler for each event. */
	    int offset = 0;
	    while (offset < avail) {
		struct inotify_event *event = (struct inotify_event*)(buffer + offset);

		event_dict = PyDict_New();
		PyDict_SetItemString(event_dict, "wd", PyInt_FromLong(event->wd));
		PyDict_SetItemString(event_dict, "mask", PyLong_FromUnsignedLong(event->mask));
		PyDict_SetItemString(event_dict, "cookie", PyLong_FromUnsignedLong(event->cookie));
		PyDict_SetItemString(event_dict, "len", PyLong_FromUnsignedLong(event->len));
		PyDict_SetItemString(event_dict, "name", PyString_FromString(event->name));

		PyObject_CallFunction(callback, "(O)", event_dict);

		Py_XDECREF(event_dict);

		offset = offset + sizeof(struct inotify_event) + event->len;
	    }

	    PyMem_Free(buffer);
	}
	Py_RETURN_NONE; 

failed:
	if ( buffer ) {
	    PyMem_Free(buffer);
	}
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
	OBJECT_ADD_ULONG(m, "ONESHOT", IN_ONESHOT);
	OBJECT_ADD_ULONG(m, "ONLYDIR", IN_ONLYDIR);
	OBJECT_ADD_ULONG(m, "IGNORED", IN_IGNORED);
	OBJECT_ADD_ULONG(m, "ISDIR", IN_ISDIR);
	OBJECT_ADD_ULONG(m, "Q_OVERFLOW", IN_Q_OVERFLOW);
	OBJECT_ADD_ULONG(m, "UNMOUNT", IN_UNMOUNT);
#undef OBJECT_ADD_ULONG
	}
}

