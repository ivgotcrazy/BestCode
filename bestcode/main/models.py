from django.db import models

# Create your models here.
class News(models.Model):
	news_text = models.TextField(max_length=4096)
	news_date = models.DateTimeField('date published')
	news_title = models.CharField(max_length=128)

	def __str__(self):
		return self.news_title

class Rules(models.Model):
	rules_name = models.CharField(max_length=128)
	rules_file = models.CharField(max_length=256)

	def __str__(self):
		return self.rules_file

class Banners(models.Model):
	banner_name = models.CharField(max_length=64)
	banner_file = models.CharField(max_length=256)
	
	def __str__(self):
		return self.banner_name	
