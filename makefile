subsystem:
		$(MAKE) -C ./GCP
		$(MAKE) -C ./GridModel
		$(MAKE) -C ./RinexTest
		$(MAKE) -C ./GridModelTest
		$(MAKE) -C ./IonosphereModeler