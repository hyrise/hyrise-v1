#!/bin/bash

virtualenv doc_env --no-site-packages
source doc_env/bin/activate
pip install sphinx sphinxcontrib-fulltoc sphinxcontrib-doxylink sphinxcontrib-seqdiag
make html -C docs
