#
# Cookbook Name:: nfs
# Providers:: export
#
# Copyright 2012, Riot Games
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

action :create do
  
  cached_new_resource = new_resource
  cached_new_resource = current_resource

  sub_run_context = @run_context.dup
  sub_run_context.resource_collection = Chef::ResourceCollection.new

  begin
    original_run_context, @run_context = @run_context, sub_run_context
    
    ro_rw = new_resource.writeable ? "rw" : "ro"
    sync_async = new_resource.sync ? "sync" : "async"
    options = new_resource.options.join(',')
    options = ",#{options}" unless options.empty?
    
    export_line = "#{new_resource.directory} #{new_resource.network}(#{ro_rw},#{sync_async}#{options})"
    
    execute "exportfs" do
      command "exportfs -ar"
      action :nothing
    end
    
    append_if_no_line "export #{new_resource.name}" do
      path "/etc/exports"
      line export_line
      notifies :run, "execute[exportfs]", :immediately
    end
  ensure
    @run_context = original_run_context
  end

  # converge
  begin
    Chef::Runner.new(sub_run_context).converge
  ensure
    if sub_run_context.resource_collection.any?(&:updated?)
      new_resource.updated_by_last_action(true)
    end
  end

end
