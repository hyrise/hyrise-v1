# NFS [![Build Status](https://secure.travis-ci.org/atomic-penguin/cookbook-nfs.png?branch=master)](http://travis-ci.org/atomic-penguin/cookbook-nfs)

## Description

Installs and configures NFS client, or server components 

## Requirements

Should work on any Red Hat-family or Debian-family Linux distribution.

## Attributes

* nfs['packages']
  - Makes a best effort to choose NFS client packages dependent on platform
  - NFS server package needs to be hardcoded for Debian/Ubuntu in the server
    recipe, or overridden in a role.

* nfs['service']
  - portmap - the portmap or rpcbind service depending on platform
  - lock - the statd or nfslock service depending on platform
  - server - the server component, nfs or nfs-kernel-server depending on platform

* nfs['config']
  - client\_templates - templates to iterate through on client systems, chosen by platform
  - server\_template - server specific template, chosen by platform

* nfs['port']
  - ['statd'] = Listen port for statd, default 32765
  - ['statd\_out'] = Outgoing port for statd, default 32766
  - ['mountd'] = Listen port for mountd, default 32767
  - ['lockd'] = Listen port for lockd, default 32768

* ~nfs['exports']~
  - Deprecated option.  No longer supported with the exports
    functionality being refactored into an LWRP.

## Usage

To install the NFS components for a client system, simply add nfs to the run\_list.

    name "base"
    description "Role applied to all systems"
    run_list [ "nfs" ]

Then in an nfs\_server.rb role that is applied to NFS servers:

    name "nfs_server"
    description "Role applied to the system that should be an NFS server."
    override_attributes(
      "nfs" => {
        "packages" => [ "portmap", "nfs-common", "nfs-kernel-server" ],
        "port" => {
          "statd" => 32765,
          "statd_out" => 32766,
          "mountd" => 32767,
          "lockd" => 32768
        },
        "exports" => [
          "/exports 10.0.0.0/8(ro,sync,no_root_squash)"
        ]
      }
    )
    run_list [ "nfs::server" ]

### nfs\_export LWRP Usage

Applications or other cookbooks can use the nfs\_export LWRP to add exports:

    nfs_export "/exports" do
      network '10.0.0.0/8'
      writeable false 
      sync true
      options ['no_root_squash']
    end

The default parameters for the nfs\_export LWRP are as follows

* directory 
  - directory you wish to export
  - defaults to resource name

* network
  - a CIDR, IP address, or wildcard (\*)
  - requires an option

* writeable
  - ro/rw export option
  - defaults to false

* sync
  - synchronous/asynchronous export option
  - defaults to true

* options
  - additional export options as an array, excluding the parameterized sync/async and ro/rw options
  - defaults to root\_squash

## nfs::undo recipe

Does your freshly kickstarted/preseeded system come with NFS, when you didn't ask for NFS?  This recipe inspired by the annoyances cookbook, will run once to remove NFS from the system.  Use a knife command to remove NFS components from your system like so.

    knife run_list add <node name> nfs::undo

## License and Author

Author: Eric G. Wolfe (<wolfe21@marshall.edu>)
Contributors: Riot Games, Sean OMeara

Copyright 2011-2012, Eric G. Wolfe
Copyright 2012, Riot Games
Copyright 2012, Sean OMeara

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
