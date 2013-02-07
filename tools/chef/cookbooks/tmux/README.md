[![Build Status](https://secure.travis-ci.org/opscode-cookbooks/tmux.png?branch=master)](http://travis-ci.org/opscode-cookbooks/tmux)

tmux
====

Installs tmux, a terminal multiplexer.

Requirements
============

Platform with a package named 'tmux'. Does a source install on RHEL
family.

Attributes
==========

* `node['tmux']['install_method']` - source or package, uses the
  appropriate recipe.
* `node['tmux']['version']` - version of tmux to download and install
  from source.
* `node['tmux']['checksum']` - sha256 checksum of the tmux tarball

Usage
=====

Use the recipe for the installation method you want to use, or set the
attribute on the node to install from that recipe and use the default
recipe. The default recipe also manages `/etc/tmux.conf`.

On RHEL family, `node['tmux']['install_method']` is set to source by
default. To install from package, the `yum::epel` recipe is required
to get the tmux package, and the attribte would need to be set
explicitly.

License and Author
==================

- Author: Joshua Timberman (<joshua@opscode.com>)

- Copyright: 2009-2012, Opscode, Inc. (<legal@opscode.com>)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
