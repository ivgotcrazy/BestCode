from django.contrib import admin

from .models import News, Rules, Banners

# Register your models here.

admin.site.register(News)
admin.site.register(Rules)
admin.site.register(Banners)
