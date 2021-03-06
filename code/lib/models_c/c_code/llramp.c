#include<Python.h>
#include<numpy/arrayobject.h>
#include<math.h>
#include<omp.h>

#define IND(a,i) *((double *)(a->data+i*a->strides[0]))

static PyObject *llramp(PyObject *self, PyObject *args, PyObject *keywds);

static PyObject *llramp(PyObject *self, PyObject *args, PyObject *keywds)
{
  PyObject *etc;
  PyArrayObject *x,*y, *rampparams;
  double x0,a,b,c,x1;
  int i;
  npy_intp dims[1];

  //etc = PyList_New(0);

  static char *kwlist[] = {"rampparams","x","etc",NULL};

  if(!PyArg_ParseTupleAndKeywords(args,keywds,"OO|O",kwlist,&rampparams,&x,&etc))
    {
      return NULL;
    }

  x0 = IND(rampparams,0);
  a  = IND(rampparams,1);
  b  = IND(rampparams,2);
  c  = IND(rampparams,3);
  x1 = IND(rampparams,4);

  dims[0] = x->dimensions[0];

  y = (PyArrayObject *) PyArray_SimpleNew(1,dims,PyArray_DOUBLE);
  #pragma omp parallel for
  for(i=0;i<dims[0];i++)
    {
      IND(y,i) = 1;
      if(IND(x,i)>x0)
	{
	  IND(y,i) = a*log(IND(x,i)-x0)+b*(IND(x,i)-x1)+c;
	}
    }

  return PyArray_Return(y);
}

static char module_docstring[]="\
  This function creates a model that fits a ramp using a log + linear ploynomial.\n\
\n\
  Parameters\n\
  ----------\n\
    x0: phase offset for log term\n\
    a:  log(x) constant\n\
    b:  x constant\n\
    c:  x=0 offset\n\
    x1: phase offset for polynomial\n\
    x:  Array of time/phase points\n\
\n\
  Returns\n\
  -------\n\
    This function returns the flux values for the ramp models\n\
\n\
  Revisions\n\
  ---------\n\
  2008-08-31	Kevin Stevenson, UCF  	\n\
                kevin218@knights.ucf.edu\n\
                Original version\n\
  2010-07-07    Kevin Stevenson\n\
                New code for when x < x0\n\
  2010-12-26    Nate Lust, UCF\n\
                natelust at linux dot com\n\
                Updated to C extension\n\
  2018-11-22    Jonathan Fraine, SSI\n\
                jfraine at spacescience.org\n\
                Updated c extensions to python3, with support for python2.7\n\
";

static PyMethodDef module_methods[] = {
  {"llramp",(PyCFunction)llramp,METH_VARARGS|METH_KEYWORDS,module_docstring},{NULL}};

PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
    PyInit_llramp(void)
#else
    initllramp(void)
#endif
{
    #if PY_MAJOR_VERSION >= 3
        PyObject *module;
        static struct PyModuleDef moduledef = {
            PyModuleDef_HEAD_INIT,
            "llramp",             /* m_name */
            module_docstring,    /* m_doc */
            -1,                  /* m_size */
            module_methods,      /* m_methods */
            NULL,                /* m_reload */
            NULL,                /* m_traverse */
            NULL,                /* m_clear */
            NULL,                /* m_free */
        };
    #endif

    #if PY_MAJOR_VERSION >= 3
        module = PyModule_Create(&moduledef);
        if (!module)
            return NULL;
        /* Load `numpy` functionality. */
        import_array();
        return module;
    #else
        PyObject *m = Py_InitModule3("llramp", module_methods, module_docstring);
        if (m == NULL)
            return;
        /* Load `numpy` functionality. */
        import_array();
    #endif
}
