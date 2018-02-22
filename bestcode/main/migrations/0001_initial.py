# -*- coding: utf-8 -*-
# Generated by Django 1.10 on 2017-08-28 04:12
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    initial = True

    dependencies = [
    ]

    operations = [
        migrations.CreateModel(
            name='Banner',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('banner_name', models.CharField(max_length=64)),
                ('banner_file', models.CharField(max_length=256)),
                ('banner_url', models.CharField(default='', max_length=256)),
            ],
        ),
        migrations.CreateModel(
            name='News',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('news_text', models.TextField(max_length=4096)),
                ('news_date', models.DateTimeField(verbose_name='date published')),
                ('news_title', models.CharField(max_length=128)),
            ],
        ),
        migrations.CreateModel(
            name='Rule',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('rule_name', models.CharField(max_length=128)),
                ('rule_file', models.CharField(max_length=256)),
            ],
        ),
    ]
