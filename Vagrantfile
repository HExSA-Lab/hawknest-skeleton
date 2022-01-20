Vagrant.configure("2") do |config|

  config.vm.box = "mrlesmithjr/fedora29-desktop"
  config.vm.synced_folder ".", "/home/vagrant/hawknest"
  config.vm.provision :shell, path: "https://gist.githubusercontent.com/khale/70ec9084752f998be71bedc978654723/raw/f235a43bac4196633c55ec1aed4458dbe28d2272/gistfile1.txt"

  config.vm.provider "vmware_desktop" do |v|
    v.gui = true
  end

  config.vm.provider "virtualbox" do |v|
    v.gui = true
  end

end
