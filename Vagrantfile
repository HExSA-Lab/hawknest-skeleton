Vagrant.configure("2") do |config|

  config.vm.box = "mrlesmithjr/fedora29-desktop"
  config.vm.synced_folder ".", "/home/vagrant/hawknest"
  config.vm.provision :shell, path: "https://gist.githubusercontent.com/khale/70ec9084752f998be71bedc978654723/raw/9406617707d62f45267ec2ea3b20e3c71a244e52/gistfile1.txt"

  config.vm.provider "vmware_desktop" do |v|
    v.gui = true
  end

  config.vm.provider "virtualbox" do |v|
    v.gui = true
  end

end
