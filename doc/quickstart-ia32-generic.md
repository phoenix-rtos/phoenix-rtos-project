Test
<section>
<h1>ia32-generic</h1>
<div class="grid">
<div class="col-9">
<p>
This version is designated for generic PC based on IA32 processor. The image consist of bootloader, kernel, TTY VGA driver, ATA driver with ext2 filesystem support and TCP/IP stack.
</p>

<p>
The easiest way to run Phoenix-RTOS directly from the image is to download and import image into the VirtualBox.
</p>

<center><img src="http://r.dcs.redcdn.pl/http/o2/phoesys/documentation/phoenix-ia32-vb-1.png" width=600></center>

<p>
After importing the image the virtual machine network settings should be changed to allow the use of the TCP/IP networking. Please change connectivity to "Host-only adapter".
</p>

<center><img src="http://r.dcs.redcdn.pl/http/o2/phoesys/documentation/phoenix-ia32-vb-2.png" width=600></center>

<p>To use Host-only adapter the virtual interface should be defined. This can be done using File/Host Network Manager option.</p>

<center><img src="http://r.dcs.redcdn.pl/http/o2/phoesys/documentation/phoenix-ia32-vb-4.png" width=600></center>

<p>After configuring the host network Phoenix-RTOS can be launched. The IP address is obtained using DHCP protocol.
Please check the IP address from command line using 'ifconfig' tool.
</p>

<center><img src="http://r.dcs.redcdn.pl/http/o2/phoesys/documentation/phoenix-ia32-vb-3.png" width=600></center>

<p>You can access Phoenix-RTOS machine using telnet.</p>

<center><img src="http://r.dcs.redcdn.pl/http/o2/phoesys/documentation/phoenix-ia32-vb-5.png" width=600></center>

      </div>
      <div class="col-3">
        <h3>Binaries</h3>
        <ul>
        <li><a href="http://r.dcs.redcdn.pl/http/o2/phoesys/quickstart/phoenix-rtos-ia32-2020.02.03.ova"><span>
phoenix-rtos-ia32-2020.02.03.ova
</span></a></li>
        <li><a href="http://r.dcs.redcdn.pl/http/o2/phoesys/quickstart/phoenix-rtos-ia32-2020.01.24.ova"><span>
phoenix-rtos-ia32-2020.01.24.ova
</span></a></li>
         </ul>
      </div>
    </div>
</section>

