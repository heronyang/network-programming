apache:
	cp apache.conf /etc/apache2/sites-available/nphw3.conf
	a2ensite nphw3.conf && service apache2 reload
	service apache2 reload

apache-rm:
	a2dissite nphw3.conf
	rm /etc/apache2/sites-available/nphw3.conf
	service apache2 reload

apache-re: apache-rm apache

install:
	apt-get install libapache2-mod-macro
	a2enmod macro
	/etc/init.d/apache2 restart

npserver:
	cp -r www/cgi-bin/ ~/public_html/
	cp www/form_get.html ~/public_html/
