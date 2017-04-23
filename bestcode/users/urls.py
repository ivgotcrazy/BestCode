from django.conf.urls import url
from . import views

urlpatterns = [
    url(r'^$', views.login, name='login'),
    url(r'^login', views.login, name='login'),
    url(r'^register', views.register, name='register'),
]

