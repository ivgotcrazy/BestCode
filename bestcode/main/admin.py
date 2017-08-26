from django.contrib import admin

from .models import News, Rule, Banner

# Register your models here.

admin.site.register(News)
admin.site.register(Rule)
admin.site.register(Banner)
